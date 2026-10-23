"""Maintain the concise route-blocker prompt for a parked FF6 party."""
import json
from pathlib import Path

CATALOG = Path(__file__).resolve().parents[1] / "data/common/mod_text.json"
LOCALES = ("en", "ja", "fr", "it", "de", "es", "ko", "zht", "zhc", "ru", "pt", "th")
OLD_ROWS = {
    "Party {0} blocks the route.": (
        "Party {0} blocks the route.", "パーティ{0}が道をふさいでいます。", "L'équipe {0} bloque le passage.",
        "Il gruppo {0} blocca il passaggio.", "Gruppe {0} versperrt den Weg.", "El grupo {0} bloquea el camino.",
        "파티 {0}가 길을 막고 있습니다.", "隊伍{0}擋住了路線。", "队伍{0}挡住了路线。", "Отряд {0} преграждает путь.",
        "O grupo {0} está bloqueando a rota.", "ปาร์ตี้ {0} ขวางเส้นทางอยู่",
    ),
    "Switch to Party {0} and move clear.": (
        "Switch to Party {0} and move clear.", "パーティ{0}に切り替えて、道を空けてください。", "Passez à l'équipe {0} et dégagez le passage.",
        "Passa al gruppo {0} e libera il passaggio.", "Wechsle zu Gruppe {0} und mache den Weg frei.", "Cambia al grupo {0} y despeja el camino.",
        "파티 {0}로 전환해 길을 비우세요.", "切換至隊伍{0}並移開。", "切换至队伍{0}并移开。", "Переключитесь на отряд {0} и освободите путь.",
        "Troque para o grupo {0} e libere a passagem.", "เปลี่ยนเป็นปาร์ตี้ {0} แล้วออกจากเส้นทาง",
    ),
}
ROWS = {
    "Blocked by Party {0}.": (
        "Blocked by Party {0}.", "パーティ{0}が道をふさいでいます。", "Bloqué par l'équipe {0}.",
        "Bloccato dal gruppo {0}.", "Von Gruppe {0} blockiert.", "Bloqueado por el grupo {0}.",
        "파티 {0}가 길을 막고 있습니다.", "遭隊伍{0}阻擋。", "被队伍{0}挡住。", "Путь преграждает отряд {0}.",
        "Bloqueado pelo grupo {0}.", "ถูกปาร์ตี้ {0} ขวาง",
    ),
}


def main():
    data = json.loads(CATALOG.read_text(encoding="utf-8"))
    changed = False
    for key, values in OLD_ROWS.items():
        if key in data:
            assert data[key] == dict(zip(LOCALES, values)), f"Refusing to remove changed catalog key: {key}"
            del data[key]
            changed = True
    for key, values in ROWS.items():
        assert len(values) == len(LOCALES)
        assert all(value.count("{0}") == key.count("{0}") for value in values)
        row = dict(zip(LOCALES, values))
        if key in data:
            assert data[key] == row, f"Refusing to replace mismatched catalog key: {key}"
        else:
            data[key] = row
            changed = True
    if changed:
        CATALOG.write_text(json.dumps(data, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
