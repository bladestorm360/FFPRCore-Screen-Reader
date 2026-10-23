"""Idempotent 12-language UI catalog for story filtering and save-point views."""
import json
from pathlib import Path

LANGS = ('en', 'ja', 'fr', 'it', 'de', 'es', 'ko', 'zht', 'zhc', 'ru', 'pt', 'th')
TEXT = {
    'Interact: {0}': (
        'Interact: {0}', '調べる: {0}', 'Interagir : {0}', 'Interagisci: {0}', 'Interagieren: {0}',
        'Interactuar: {0}', '상호작용: {0}', '互動：{0}', '互动：{0}', 'Взаимодействовать: {0}',
        'Interagir: {0}', 'โต้ตอบ: {0}'),
    'Boss part: Shell': (
        'Shell', '殻', 'Coquille', 'Guscio', 'Panzer', 'Caparazón', '껍질', '外殼', '外壳', 'Панцирь', 'Casco', 'เปลือก'),
    'Boss part: Head': (
        'Head', '頭部', 'Tête', 'Testa', 'Kopf', 'Cabeza', '머리', '頭部', '头部', 'Голова', 'Cabeça', 'หัว'),
    'Boss part: Goddess': (
        'Goddess', '女神', 'Déesse', 'Dea', 'Göttin', 'Diosa', '여신', '女神', '女神', 'Богиня', 'Deusa', 'เทพธิดา'),
    'Boss part: Demon': (
        'Demon', '悪霊', 'Démon', 'Demone', 'Dämon', 'Demonio', '악령', '惡魔', '恶魔', 'Демон', 'Demônio', 'ปีศาจ'),
    'Hide story-gated destinations': (
        'Hide story-gated destinations', 'ストーリー進行で制限された目的地を非表示',
        'Masquer les destinations bloquées par le scénario', 'Nascondi le destinazioni bloccate dalla storia',
        'Durch die Handlung gesperrte Ziele ausblenden', 'Ocultar destinos bloqueados por la historia',
        '스토리 진행으로 막힌 목적지 숨기기', '隱藏因劇情封鎖的目的地', '隐藏因剧情封锁的目的地',
        'Скрывать цели, закрытые по сюжету', 'Ocultar destinos bloqueados pela história',
        'ซ่อนจุดหมายที่ถูกปิดกั้นตามเนื้อเรื่อง'),
    'On. Destinations blocked by confirmed story gates are hidden. Uncertain routes remain listed.': (
        'On. Destinations blocked by confirmed story gates are hidden. Uncertain routes remain listed.',
        'オン。ストーリー進行による障害が確認された目的地を非表示にします。経路が不確かな目的地は表示します。',
        'Activé. Les destinations dont le blocage par le scénario est confirmé sont masquées. Celles dont le trajet est incertain restent affichées.',
        'Attivo. Nasconde le destinazioni con un blocco narrativo confermato. I percorsi incerti restano elencati.',
        'Ein. Ziele mit bestätigten Handlungssperren werden ausgeblendet. Unklare Routen bleiben aufgelistet.',
        'Activado. Se ocultan los destinos con un bloqueo de historia confirmado. Las rutas inciertas siguen en la lista.',
        '켬. 스토리 진행 장벽으로 막힌 것이 확인된 목적지를 숨깁니다. 경로가 불확실한 목적지는 목록에 남습니다.',
        '開啟。隱藏確認因劇情障礙而無法抵達的目的地。路線不確定的目的地仍會列出。',
        '开启。隐藏确认因剧情障碍而无法抵达的目的地。路线不确定的目的地仍会列出。',
        'Включено. Цели с подтверждённой сюжетной преградой скрыты. Неопределённые маршруты остаются в списке.',
        'Ativado. Destinos com bloqueios de história confirmados são ocultados. Rotas incertas continuam na lista.',
        'เปิด ซ่อนจุดหมายที่ยืนยันว่าถูกปิดกั้นตามเนื้อเรื่อง จุดหมายที่ยังไม่แน่ใจเรื่องเส้นทางจะยังอยู่ในรายการ'),
    'Off. Story-gated destinations remain listed in navigation.': (
        'Off. Story-gated destinations remain listed in navigation.', 'オフ。ストーリー進行で制限された目的地もナビゲーションに表示します。',
        'Désactivé. Les destinations bloquées par le scénario restent dans la liste de navigation.',
        'Disattivo. Le destinazioni bloccate dalla storia restano nella navigazione.',
        'Aus. Durch die Handlung gesperrte Ziele bleiben in der Navigation aufgelistet.',
        'Desactivado. Los destinos bloqueados por la historia siguen en la lista de navegación.',
        '끔. 스토리 진행으로 막힌 목적지도 탐색 목록에 표시합니다.',
        '關閉。因劇情封鎖的目的地仍會列在導覽中。', '关闭。因剧情封锁的目的地仍会列在导航中。',
        'Выключено. Закрытые по сюжету цели остаются в списке навигации.',
        'Desativado. Destinos bloqueados pela história continuam na lista de navegação.',
        'ปิด จุดหมายที่ถูกปิดกั้นตามเนื้อเรื่องจะยังอยู่ในรายการนำทาง'),
    'Toggle story gate filter': (
        'Toggle story gate filter', 'ストーリー進行による目的地フィルターの切り替え',
        'Activer ou désactiver le filtre des blocages du scénario', 'Attiva o disattiva il filtro dei blocchi narrativi',
        'Filter für Handlungssperren umschalten', 'Alternar filtro de bloqueos de historia',
        '스토리 장벽 필터 전환', '切換劇情封鎖篩選', '切换剧情封锁筛选',
        'Переключить фильтр сюжетных преград', 'Alternar filtro de bloqueios de história',
        'สลับตัวกรองจุดหมายที่ถูกปิดกั้นตามเนื้อเรื่อง'),
    'Save Points': (
        'Save Points', 'セーブポイント', 'Points de sauvegarde', 'Punti di salvataggio', 'Speicherpunkte',
        'Puntos de guardado', '저장 지점', '存檔點', '存档点', 'Точки сохранения', 'Pontos de salvamento', 'จุดบันทึก'),
}

if __name__ == '__main__':
    path = Path(__file__).resolve().parents[1] / 'data/common/mod_text.json'
    catalog = json.loads(path.read_text(encoding='utf8'))
    for key, values in TEXT.items():
        assert len(values) == len(LANGS) and all(values), key
        catalog[key] = dict(zip(LANGS, values))
    path.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + '\n', encoding='utf8', newline='\n')
    print(f'Navigation filters: {len(TEXT)} entries x {len(LANGS)} languages')
