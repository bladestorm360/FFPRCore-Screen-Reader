"""Add concise FF6 story navigation prompts to the shared localization catalog."""
import json
from pathlib import Path

CATALOG = Path(__file__).resolve().parents[1] / "data/common/mod_text.json"
LOCALES = ("en", "ja", "fr", "it", "de", "es", "ko", "zht", "zhc", "ru", "pt", "th")

# Values are ordered by LOCALES. Keep format placeholders unchanged.
ROWS = {
"Banon in danger.": ("Banon in danger.","バナンが危ない。","Banon est en danger.","Banon è in pericolo.","Banon ist in Gefahr.","Banon está en peligro.","바넌이 위험합니다.","巴南有危險。","巴南有危险。","Банон в опасности.","Banon está em perigo.","บานอนกำลังตกอยู่ในอันตราย"),
"Continue through the passage.": ("Continue through the passage.","通路を進んでください。","Continuez dans le passage.","Prosegui nel passaggio.","Gehe durch den Gang weiter.","Continúa por el pasadizo.","통로를 계속 진행하세요.","繼續穿過通道。","继续穿过通道。","Продолжайте идти по проходу.","Continue pelo corredor.","เดินต่อไปตามทาง"),
"Enter the stage.": ("Enter the stage.","舞台に入ってください。","Entrez sur la scène.","Entra in scena.","Betritt die Bühne.","Entra en el escenario.","무대로 들어가세요.","進入舞台。","进入舞台。","Выйдите на сцену.","Entre no palco.","เข้าไปบนเวที"),
"Fight {0}.": ("Fight {0}.","{0}と戦ってください。","Affrontez {0}.","Affronta {0}.","Kämpfe gegen {0}.","Lucha contra {0}.","{0}와 싸우세요.","與{0}戰鬥。","与{0}战斗。","Сразитесь с {0}.","Lute contra {0}.","ต่อสู้กับ {0}"),
"Follow performance cues.": ("Follow performance cues.","演技の合図に従ってください。","Suivez les indications de la représentation.","Segui le indicazioni dello spettacolo.","Folge den Regieanweisungen.","Sigue las indicaciones de la función.","공연 신호를 따르세요.","按照演出提示行動。","按照演出提示行动。","Следуйте подсказкам представления.","Siga as deixas da apresentação.","ทำตามสัญญาณการแสดง"),
"Hold switch. Switch parties.": ("Hold switch. Switch parties.","スイッチを押さえたまま、パーティを切り替えてください。","Maintenez l'interrupteur, puis changez d'équipe.","Tieni premuto l'interruttore e cambia gruppo.","Halte den Schalter gedrückt und wechsle die Gruppe.","Mantén pulsado el interruptor y cambia de grupo.","스위치를 누른 채 파티를 전환하세요.","壓住開關，再切換隊伍。","压住开关，再切换队伍。","Удерживайте переключатель и смените отряд.","Mantenha o interruptor pressionado e troque de grupo.","เหยียบสวิตช์ค้างไว้ แล้วเปลี่ยนปาร์ตี้"),
"Join {0}.": ("Join {0}.","{0}に加わってください。","Rejoignez {0}.","Unisciti a {0}.","Schließe dich {0} an.","Únete a {0}.","{0}에 합류하세요.","加入{0}。","加入{0}。","Присоединитесь к {0}.","Junte-se a {0}.","ไปรวมกับ{0}"),
"Leave the switch.": ("Leave the switch.","スイッチから離れてください。","Quittez l'interrupteur.","Lascia l'interruttore.","Verlasse den Schalter.","Deja el interruptor.","스위치에서 벗어나세요.","離開開關。","离开开关。","Отойдите от переключателя.","Saia do interruptor.","ออกจากสวิตช์"),
"Operate {0}.": ("Operate {0}.","{0}を操作してください。","Actionnez {0}.","Aziona {0}.","Betätige {0}.","Activa {0}.","{0}을(를) 조작하세요.","操作{0}。","操作{0}。","Используйте {0}.","Acione {0}.","ใช้งาน{0}"),
"Party {0} closest.": ("Party {0} closest.","パーティ{0}が最も近くにいます。","L'équipe {0} est la plus proche.","Il gruppo {0} è il più vicino.","Gruppe {0} ist am nächsten.","El grupo {0} es el más cercano.","파티 {0}가 가장 가깝습니다.","隊伍{0}距離最近。","队伍{0}距离最近。","Отряд {0} ближе всех.","O grupo {0} está mais perto.","ปาร์ตี้ {0} อยู่ใกล้ที่สุด"),
"Pull the far-right lever.": ("Pull the far-right lever.","一番右のレバーを引いてください。","Tirez le levier tout à droite.","Tira la leva più a destra.","Ziehe den Hebel ganz rechts.","Tira de la palanca de más a la derecha.","맨 오른쪽 레버를 당기세요.","拉動最右側的拉桿。","拉动最右侧的拉杆。","Потяните за крайний правый рычаг.","Puxe a alavanca mais à direita.","ดึงคันโยกขวาสุด"),
"Push {0}. Clear the party below first.": ("Push {0}. Clear the party below first.","{0}を押してください。その前に下のパーティを移動させてください。","Poussez {0}. Éloignez d'abord l'équipe en dessous.","Spingi {0}. Prima sposta il gruppo sottostante.","Schiebe {0}. Bringe zuerst die Gruppe darunter in Sicherheit.","Empuja {0}. Aparta primero al grupo de abajo.","{0}을(를) 미세요. 먼저 아래 파티를 비우세요.","推動{0}。先讓下方隊伍離開。","推动{0}。先让下方队伍离开。","Нажмите {0}. Сначала отведите нижний отряд.","Empurre {0}. Afaste primeiro o grupo de baixo.","ผลัก{0} ก่อนอื่นย้ายปาร์ตี้ด้านล่างออกไป"),
"Reach {0}.": ("Reach {0}.","{0}に向かってください。","Rejoignez {0}.","Raggiungi {0}.","Erreiche {0}.","Llega hasta {0}.","{0}에 도달하세요.","前往{0}。","前往{0}。","Доберитесь до {0}.","Chegue até {0}.","ไปให้ถึง{0}"),
"Read {0}.": ("Read {0}.","{0}を読んでください。","Lisez {0}.","Leggi {0}.","Lies {0}.","Lee {0}.","{0}을(를) 읽으세요.","閱讀{0}。","阅读{0}。","Прочитайте {0}.","Leia {0}.","อ่าน{0}"),
"Step on {0}.": ("Step on {0}.","{0}に乗ってください。","Marchez sur {0}.","Sali su {0}.","Tritt auf {0}.","Pisa {0}.","{0}을(를) 밟으세요.","踩上{0}。","踩上{0}。","Наступите на {0}.","Pise em {0}.","เหยียบ{0}"),
"Talk to {0}.": ("Talk to {0}.","{0}に話しかけてください。","Parlez à {0}.","Parla con {0}.","Sprich mit {0}.","Habla con {0}.","{0}에게 말을 거세요.","與{0}交談。","与{0}交谈。","Поговорите с {0}.","Fale com {0}.","คุยกับ{0}"),
"Terra in danger.": ("Terra in danger.","ティナが危ない。","Terra est en danger.","Terra è in pericolo.","Terra ist in Gefahr.","Terra está en peligro.","티나가 위험합니다.","蒂娜有危險。","蒂娜有危险。","Терра в опасности.","Terra está em perigo.","เทอร์รากำลังตกอยู่ในอันตราย"),
}

def main():
    data = json.loads(CATALOG.read_text(encoding="utf-8"))
    assert len(ROWS) == 17
    assert set(ROWS) <= {"Banon in danger.", "Continue through the passage.", "Enter the stage.", "Fight {0}.", "Follow performance cues.", "Hold switch. Switch parties.", "Join {0}.", "Leave the switch.", "Operate {0}.", "Party {0} closest.", "Pull the far-right lever.", "Push {0}. Clear the party below first.", "Reach {0}.", "Read {0}.", "Step on {0}.", "Talk to {0}.", "Terra in danger."}
    # Correct the three previously added name spellings, but only from their
    # exact known old values so unrelated catalog changes cannot be overwritten.
    corrections = {
        ("Banon in danger.", "ko"): ("바난이 위험합니다.", "바넌이 위험합니다."),
        ("Terra in danger.", "ru"): ("Тина в опасности.", "Терра в опасности."),
        ("Terra in danger.", "th"): ("ทีน่ากำลังตกอยู่ในอันตราย", "เทอร์รากำลังตกอยู่ในอันตราย"),
    }
    changed = False
    for (key, locale), (old, new) in corrections.items():
        if data[key][locale] == old:
            data[key][locale] = new
            changed = True
        else:
            assert data[key][locale] == new, f"Refusing unexpected value for {key}/{locale}"
    for key, values in ROWS.items():
        assert len(values) == len(LOCALES)
        assert values[0] == key
        for value in values:
            assert value.count("{0}") == key.count("{0}"), (key, value)
        row = dict(zip(LOCALES, values))
        if key in data:
            for locale, value in row.items():
                assert data[key][locale] == value, f"Refusing to replace mismatched catalog value: {key}/{locale}"
        else:
            data[key] = row
            changed = True
    if changed:
        CATALOG.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")

if __name__ == "__main__":
    main()
