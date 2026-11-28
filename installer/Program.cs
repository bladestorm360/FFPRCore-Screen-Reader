using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;

namespace FFPR.Setup
{
    internal static class Program
    {
        [STAThread]
        private static void Main()
        {
            try { SetupRuntime.Initialize(); }
            catch (Win32Exception error)
            {
                MessageBox.Show(error.Message, "FFPR Screen Reader Setup", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new SetupForm());
        }
    }

    internal sealed class SetupForm : Form
    {
        private readonly CheckedListBox games = new CheckedListBox { CheckOnClick = true, Dock = DockStyle.Fill };
        private readonly CheckBox all = new CheckBox { Text = "Select &all detected games", AutoSize = true };
        private readonly Button browse = new Button { Text = "Add Steam &library folder", AutoSize = true };
        private readonly TextBox output = new TextBox {
            AccessibleName = "Setup results", Multiline = true, ReadOnly = true, ScrollBars = ScrollBars.Vertical, Dock = DockStyle.Fill
        };
        private readonly Label status = new Label { AutoSize = true };
        private readonly FlowLayoutPanel commands = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true };
        private readonly List<string> roots = GameCatalog.SteamRoots().ToList();
        private bool busy, changingAll;

        internal SetupForm()
        {
            Text = "Final Fantasy Pixel Remaster Screen Reader Setup";
            AccessibleName = Text;
            AutoScaleMode = AutoScaleMode.Font;
            Font = SystemFonts.MessageBoxFont;
            MinimumSize = new Size(720, 480);
            Size = new Size(860, 580);
            StartPosition = FormStartPosition.CenterScreen;
            games.AccessibleName = "Detected Steam games. Press Space to select or clear a game.";
            var layout = new TableLayoutPanel {
                Dock = DockStyle.Fill, ColumnCount = 1, RowCount = 6, Padding = new Padding(12)
            };
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.RowStyles.Add(new RowStyle(SizeType.Percent, 45));
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.RowStyles.Add(new RowStyle(SizeType.Percent, 55));
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.Controls.Add(new Label { Text = "Detected &games", AutoSize = true }, 0, 0);
            layout.Controls.Add(games, 0, 1);
            var selection = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true };
            selection.Controls.Add(all);
            selection.Controls.Add(browse);
            layout.Controls.Add(selection, 0, 2);
            AddCommand("&Check for updates", SetupAction.Check);
            AddCommand("&Install or update", SetupAction.Install);
            AddCommand("&Rollback", SetupAction.Rollback);
            AddCommand("&Uninstall", SetupAction.Uninstall);
            layout.Controls.Add(commands, 0, 3);
            layout.Controls.Add(output, 0, 4);
            layout.Controls.Add(status, 0, 5);
            Controls.Add(layout);

            all.CheckedChanged += delegate
            {
                if (changingAll) return;
                for (int i = 0; i < games.Items.Count; ++i) games.SetItemChecked(i, all.Checked);
            };
            games.ItemCheck += delegate
            {
                BeginInvoke(new MethodInvoker(UpdateSelection));
            };
            browse.Click += delegate
            {
                using (var dialog = new FolderBrowserDialog {
                    Description = "Choose the Steam library folder that contains steamapps.", ShowNewFolderButton = false
                })
                    if (dialog.ShowDialog(this) == DialogResult.OK)
                    {
                        roots.Add(dialog.SelectedPath);
                        Discover();
                    }
            };
            Shown += delegate { Discover(); };
            FormClosing += delegate(object sender, FormClosingEventArgs args)
            {
                if (busy && args.CloseReason == CloseReason.UserClosing) args.Cancel = true;
            };
        }

        private void UpdateSelection()
        {
            changingAll = true;
            all.Checked = games.Items.Count > 0 && games.CheckedItems.Count == games.Items.Count;
            changingAll = false;
            all.Enabled = !busy && games.Items.Count > 0;
            commands.Enabled = !busy && games.CheckedItems.Count > 0;
        }

        private void AddCommand(string title, SetupAction action)
        {
            var button = new Button { Text = title, AutoSize = true };
            button.Click += delegate { Start(action); };
            commands.Controls.Add(button);
        }

        private void Discover()
        {
            var selected = new HashSet<string>(games.CheckedItems.Cast<GameTarget>().Select(game => game.Root),
                                               StringComparer.OrdinalIgnoreCase);
            string[] locations = roots.ToArray();
            BeginWork("Finding Steam games.", worker => GameCatalog.Find(locations), result =>
            {
                games.Items.Clear();
                foreach (var game in (List<GameTarget>)result) games.Items.Add(game, selected.Contains(game.Root));
                UpdateSelection();
                status.Text = games.Items.Count == 0
                    ? "No games found. Try Add Steam library folder."
                    : games.Items.Count + " installations found. Select the games to manage.";
                output.Text = "Close the selected games before installing, rolling back, or uninstalling. " +
                              "Setup preserves settings, saves, and waypoints.";
                if (games.Items.Count > 0) games.Focus();
                else browse.Focus();
            });
        }

        private void Start(SetupAction action)
        {
            var selected = games.CheckedItems.Cast<GameTarget>().ToList();
            if (selected.Count == 0) return;
            if (action == SetupAction.Rollback || action == SetupAction.Uninstall)
            {
                string verb = action == SetupAction.Rollback ? "Roll back setup changes in these games?" : "Uninstall the screen reader from these games?";
                string text = verb + Environment.NewLine + Environment.NewLine + String.Join(Environment.NewLine, selected);
                if (MessageBox.Show(this, text, Text, MessageBoxButtons.YesNo, MessageBoxIcon.Question,
                                    MessageBoxDefaultButton.Button2) != DialogResult.Yes) return;
            }
            BeginWork("Working. Please keep the selected games closed.", worker =>
            {
                SetupActions.Run(action, selected, line => worker.ReportProgress(0, line), ReleaseSource.Check, ReleaseSource.Fetch);
                return null;
            }, _ =>
            {
                status.Text = "Finished. The results are ready to read.";
                output.SelectionStart = 0;
                output.SelectionLength = 0;
                output.ScrollToCaret();
                output.Focus();
            });
        }

        private void BeginWork(string message, Func<BackgroundWorker, object> run, Action<object> finish)
        {
            if (busy) return;
            SetBusy(true);
            output.Clear();
            status.Text = message;
            var worker = new BackgroundWorker { WorkerReportsProgress = true };
            worker.DoWork += (sender, args) => args.Result = run(worker);
            worker.ProgressChanged += (sender, args) => output.AppendText((string)args.UserState + Environment.NewLine);
            worker.RunWorkerCompleted += delegate(object sender, RunWorkerCompletedEventArgs args)
            {
                SetBusy(false);
                worker.Dispose();
                if (args.Error == null) finish(args.Result);
                else
                {
                    output.AppendText(args.Error.Message + Environment.NewLine);
                    status.Text = "Setup could not complete this request.";
                    output.SelectionStart = 0;
                    output.Focus();
                }
            };
            worker.RunWorkerAsync();
        }

        private void SetBusy(bool value)
        {
            busy = value;
            games.Enabled = browse.Enabled = !value;
            UpdateSelection();
        }
    }
}
