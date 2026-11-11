# Ordinary FFV doors and hidden passages

This slice covers the Bal treasury and basement, the pirate hideout door,
the Phantom Village box, the Ronka switch and the two Rift doors. Each
passage uses one ordinary control and the native state governs what lies
beyond it. None needs a new Shift+G guide. The source check pins each
contact and the corresponding native initializer.

The Bal treasury uses one area-local latch. The basement door opens from
the Jachol side and retains its locked dialogue from the castle side. The
Phantom Village box clears its local obstacle. The pirate door and Ronka
switch each use their own persistent receipt. The two Rift doors share
their original receipt. The focused checks distinguish route access to the
Bal basement door from access to the passage beyond it.

Ten assertions over three route legs passed. All seven before-and-after
state checks matched the native initializers. The ten source dispositions
and 117 canonical producer guards passed. The production data already
represents these passages correctly. No production change was needed in
this slice. 88 FFV discovery leads remain in the broader audit.
