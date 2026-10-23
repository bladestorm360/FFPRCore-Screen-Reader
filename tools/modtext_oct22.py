"""Mod-owned October22 settings/cue translations and corrected existing help.

Applied to the merge recipe, so rebuilding catalogs cannot restore obsolete
buffer-removal/floor-only wording. These are translations, not extracted game text.
"""

KEYS = (
    "Interrupt speech on character turn",
    "On. A character's turn announcement interrupts current speech.",
    "Off. A character's turn announcement queues after current speech.",
    "Enemy status announcements",
    "On. Announce visually displayed enemy conditions.",
    "Off. Do not automatically announce enemy conditions.",
    "Interaction available: {0}",
    "Battle results: notifications once on",
    "Battle results: notifications once off",
    "On. Separate entrances leading to the same floor have letter suffixes, such as (a) and (b).",
    "Off. Floor entrance names have no letter suffixes.",
)
ROWS = {
    "en": (
        "Interrupt speech on character turn",
        "On. A character's turn announcement interrupts current speech.",
        "Off. A character's turn announcement queues after current speech.",
        "Enemy status announcements",
        "On. Announce visually displayed enemy conditions.",
        "Off. Do not automatically announce enemy conditions.",
        "Interaction available: {0}",
        "On. Each result notification is automatically spoken only once per battle. All displayed notifications remain in the review buffer.",
        "Off. Result notifications are spoken on each results screen where displayed. Review buffers are unchanged.",
        "On. Distinct exits sharing a destination name have stable letters, such as (A) and (B).",
        "Off. Exit names have no letter suffixes.",
    ),
    "ja": (
        "キャラクターのターンで読み上げを中断",
        "オン。キャラクターのターン通知が現在の読み上げを中断します。",
        "オフ。キャラクターのターン通知は現在の読み上げの後に続きます。",
        "敵の状態の読み上げ",
        "オン。画面に表示された敵の状態を読み上げます。",
        "オフ。敵の状態を自動で読み上げません。",
        "操作可能: {0}",
        "オン。各リザルト通知は戦闘ごとに一度だけ自動で読み上げます。表示された通知はすべてレビューバッファに残ります。",
        "オフ。通知が表示される各リザルト画面で読み上げます。レビューバッファは変わりません。",
        "オン。同じ行き先名の異なる出口に、(A)、(B)などの固定の文字を付けます。",
        "オフ。出口名に文字を付けません。",
    ),
    "fr": (
        "Interrompre la parole au tour d'un personnage",
        "Activé. L'annonce du tour d'un personnage interrompt la parole en cours.",
        "Désactivé. L'annonce du tour d'un personnage suit la parole en cours.",
        "Annonce des états ennemis",
        "Activé. Annonce les états des ennemis affichés à l'écran.",
        "Désactivé. N'annonce pas automatiquement les états ennemis.",
        "Interaction possible : {0}",
        "Activé. Chaque notification de résultat n'est annoncée automatiquement qu'une fois par combat. Toutes restent dans le tampon de lecture.",
        "Désactivé. Les notifications sont lues sur chaque écran de résultats où elles apparaissent. Les tampons de lecture restent inchangés.",
        "Activé. Les sorties distinctes portant le même nom de destination reçoivent des lettres fixes, comme (A) et (B).",
        "Désactivé. Les noms des sorties n'ont pas de suffixe alphabétique.",
    ),
    "it": (
        "Interrompi la voce al turno del personaggio",
        "Attivo. L'annuncio del turno di un personaggio interrompe la voce in corso.",
        "Disattivo. L'annuncio del turno viene letto dopo la voce in corso.",
        "Annunci degli stati dei nemici",
        "Attivo. Annuncia gli stati dei nemici mostrati sullo schermo.",
        "Disattivo. Non annuncia automaticamente gli stati dei nemici.",
        "Interazione disponibile: {0}",
        "Attivo. Ogni notifica dei risultati viene letta automaticamente una sola volta per battaglia. Tutte restano nel buffer di lettura.",
        "Disattivo. Le notifiche vengono lette in ogni schermata dei risultati in cui appaiono. I buffer di lettura restano invariati.",
        "Attivo. Uscite distinte con lo stesso nome di destinazione ricevono lettere stabili, come (A) e (B).",
        "Disattivo. I nomi delle uscite non hanno suffissi alfabetici.",
    ),
    "de": (
        "Sprachausgabe bei Charakterzug unterbrechen",
        "Ein. Die Zugansage eines Charakters unterbricht die laufende Sprachausgabe.",
        "Aus. Die Zugansage folgt nach der laufenden Sprachausgabe.",
        "Gegnerzustände ansagen",
        "Ein. Sagt sichtbar dargestellte Gegnerzustände an.",
        "Aus. Sagt Gegnerzustände nicht automatisch an.",
        "Interaktion möglich: {0}",
        "Ein. Jede Ergebnismeldung wird pro Kampf nur einmal automatisch angesagt. Alle angezeigten Meldungen bleiben im Lesepuffer.",
        "Aus. Meldungen werden auf jedem Ergebnisbildschirm angesagt, auf dem sie erscheinen. Die Lesepuffer bleiben unverändert.",
        "Ein. Verschiedene Ausgänge mit gleichem Zielnamen erhalten feste Buchstaben wie (A) und (B).",
        "Aus. Ausgangsnamen erhalten keine Buchstabenzusätze.",
    ),
    "es": (
        "Interrumpir la voz al llegar el turno de un personaje",
        "Activado. El anuncio del turno de un personaje interrumpe la voz actual.",
        "Desactivado. El anuncio del turno espera a que termine la voz actual.",
        "Anuncios de estados enemigos",
        "Activado. Anuncia los estados enemigos mostrados en pantalla.",
        "Desactivado. No anuncia automáticamente los estados enemigos.",
        "Interacción disponible: {0}",
        "Activado. Cada aviso de resultados se lee automáticamente una sola vez por combate. Todos los avisos mostrados permanecen en el búfer de lectura.",
        "Desactivado. Los avisos se leen en cada pantalla de resultados donde aparecen. Los búferes de lectura no cambian.",
        "Activado. Las salidas distintas con el mismo nombre de destino reciben letras estables, como (A) y (B).",
        "Desactivado. Los nombres de las salidas no llevan sufijos de letras.",
    ),
    "ko": (
        "캐릭터 턴 시작 시 음성 끊기",
        "켜짐. 캐릭터의 턴 알림이 현재 음성을 끊습니다.",
        "꺼짐. 캐릭터의 턴 알림이 현재 음성 뒤에 이어집니다.",
        "적 상태 알림",
        "켜짐. 화면에 표시된 적의 상태를 알립니다.",
        "꺼짐. 적의 상태를 자동으로 알리지 않습니다.",
        "상호작용 가능: {0}",
        "켜짐. 각 결과 알림은 전투마다 한 번만 자동으로 읽습니다. 표시된 모든 알림은 읽기 버퍼에 남습니다.",
        "꺼짐. 알림이 표시되는 각 결과 화면에서 읽습니다. 읽기 버퍼는 변경되지 않습니다.",
        "켜짐. 목적지 이름이 같은 서로 다른 출구에 (A), (B) 같은 고정 문자를 붙입니다.",
        "꺼짐. 출구 이름에 문자를 붙이지 않습니다.",
    ),
    "zht": (
        "角色回合開始時中斷語音",
        "開啟。角色回合通知會中斷目前的語音。",
        "關閉。角色回合通知會接在目前語音之後。",
        "敵人狀態播報",
        "開啟。播報畫面顯示的敵人狀態。",
        "關閉。不自動播報敵人狀態。",
        "可互動：{0}",
        "開啟。每則結果通知每場戰鬥僅自動播報一次。所有已顯示通知仍保留在檢閱緩衝區。",
        "關閉。每個顯示通知的結果畫面都會播報。檢閱緩衝區不變。",
        "開啟。目的地名稱相同的不同出口會加上固定字母，例如 (A) 和 (B)。",
        "關閉。出口名稱不加字母後綴。",
    ),
    "zhc": (
        "角色回合开始时中断语音",
        "开启。角色回合通知会中断当前语音。",
        "关闭。角色回合通知会接在当前语音之后。",
        "敌人状态播报",
        "开启。播报画面显示的敌人状态。",
        "关闭。不自动播报敌人状态。",
        "可互动：{0}",
        "开启。每条结果通知每场战斗仅自动播报一次。所有已显示通知仍保留在检阅缓冲区。",
        "关闭。每个显示通知的结果画面都会播报。检阅缓冲区不变。",
        "开启。目的地名称相同的不同出口会加上固定字母，例如 (A) 和 (B)。",
        "关闭。出口名称不加字母后缀。",
    ),
    "ru": (
        "Прерывать речь при начале хода персонажа",
        "Включено. Объявление хода персонажа прерывает текущую речь.",
        "Выключено. Объявление хода персонажа следует после текущей речи.",
        "Объявления состояний врагов",
        "Включено. Объявлять показанные на экране состояния врагов.",
        "Выключено. Не объявлять состояния врагов автоматически.",
        "Доступно взаимодействие: {0}",
        "Включено. Каждое уведомление результатов автоматически читается только раз за бой. Все показанные уведомления остаются в буфере просмотра.",
        "Выключено. Уведомления читаются на каждом экране результатов, где показаны. Буферы просмотра не меняются.",
        "Включено. Разные выходы с одинаковым названием назначения получают постоянные буквы, например (A) и (B).",
        "Выключено. К названиям выходов не добавляются буквы.",
    ),
    "th": (
        "ขัดจังหวะเสียงเมื่อเริ่มเทิร์นตัวละคร",
        "เปิด การประกาศเทิร์นตัวละครจะขัดจังหวะเสียงที่กำลังอ่าน",
        "ปิด การประกาศเทิร์นตัวละครจะอ่านต่อจากเสียงปัจจุบัน",
        "ประกาศสถานะศัตรู",
        "เปิด ประกาศสถานะศัตรูที่แสดงบนหน้าจอ",
        "ปิด ไม่ประกาศสถานะศัตรูโดยอัตโนมัติ",
        "โต้ตอบได้: {0}",
        "เปิด อ่านแต่ละการแจ้งผลโดยอัตโนมัติเพียงครั้งเดียวต่อการต่อสู้ การแจ้งผลที่แสดงทั้งหมดยังคงอยู่ในบัฟเฟอร์ทบทวน",
        "ปิด อ่านการแจ้งผลในทุกหน้าผลลัพธ์ที่แสดง บัฟเฟอร์ทบทวนไม่เปลี่ยนแปลง",
        "เปิด ทางออกต่างกันที่ใช้ชื่อปลายทางเดียวกันจะมีตัวอักษรคงที่ เช่น (A) และ (B)",
        "ปิด ไม่เติมตัวอักษรท้ายชื่อทางออก",
    ),
    "pt": (
        "Interromper a fala no turno do personagem",
        "Ativado. O anúncio do turno de um personagem interrompe a fala atual.",
        "Desativado. O anúncio do turno vem depois da fala atual.",
        "Anúncios de estados dos inimigos",
        "Ativado. Anuncia os estados dos inimigos exibidos na tela.",
        "Desativado. Não anuncia automaticamente os estados dos inimigos.",
        "Interação disponível: {0}",
        "Ativado. Cada aviso de resultados é lido automaticamente apenas uma vez por batalha. Todos os avisos exibidos permanecem no buffer de leitura.",
        "Desativado. Os avisos são lidos em cada tela de resultados em que aparecem. Os buffers de leitura não mudam.",
        "Ativado. Saídas distintas com o mesmo nome de destino recebem letras fixas, como (A) e (B).",
        "Desativado. Os nomes das saídas não recebem sufixos de letras.",
    ),
}


def extend(recipes):
    """Update four existing additions and append seven keys, without duplicates."""
    assert len(ROWS) == 12 and all(len(row) == len(KEYS) for row in ROWS.values())
    for index, key in enumerate(KEYS):
        entry = {lang: row[index] for lang, row in ROWS.items()}
        recipe = ("common", key, entry, "October22 mod-owned accessibility settings/cue, translated by Sol")
        found = [i for i, row in enumerate(recipes) if row[0] == "common" and row[1] == key]
        if found:
            assert len(found) == 1
            recipes[found[0]] = recipe
        else:
            recipes.append(recipe)
    recipes.append(("common", "Iron gate", {
        "en": "Iron gate", "ja": "鉄格子", "fr": "Grille en fer", "it": "Cancello di ferro",
        "de": "Eisentor", "es": "Reja de hierro", "ko": "철문", "zht": "鐵門", "zhc": "铁门",
        "ru": "Железные ворота", "th": "ประตูเหล็ก", "pt": "Portão de ferro",
    }, "October22 FF2 interaction identity; mod-owned label translated by Sol"))

    recipes.append(("common", "Status effect", {
        "en": "Status effect", "ja": "状態変化のエフェクト", "fr": "Effet d’état", "it": "Effetto di stato",
        "de": "Zustandseffekt", "es": "Efecto de estado", "ko": "상태 효과", "zht": "狀態效果", "zhc": "状态效果",
        "ru": "Эффект состояния", "th": "เอฟเฟกต์สถานะ", "pt": "Efeito de estado",
    }, "Visible but non-unique enemy condition effect; no hidden exact condition disclosure"))
    from modtext_object_labels import extend as extend_objects
    extend_objects(recipes)
    from modtext_object_followup import extend as extend_followup
    extend_followup(recipes)
