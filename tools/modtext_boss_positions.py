"""Add localized anatomical target positions for FF5 Neo Exdeath."""
import json
from pathlib import Path

CATALOG = Path(__file__).resolve().parents[1] / "data/common/mod_text.json"
LOCALES = ("en", "ja", "fr", "it", "de", "es", "ko", "zht", "zhc", "ru", "pt", "th")
ROWS = {
    "Boss part: Center": ("Center", "中央", "Centre", "Centro", "Mitte", "Centro", "중앙", "中央", "中央", "Центр", "Centro", "ตรงกลาง"),
    "Boss part: Lower right": ("Lower right", "右下", "En bas à droite", "In basso a destra", "Unten rechts", "Abajo a la derecha", "오른쪽 아래", "右下", "右下", "Справа внизу", "Embaixo à direita", "ล่างขวา"),
    "Boss part: Upper right": ("Upper right", "右上", "En haut à droite", "In alto a destra", "Oben rechts", "Arriba a la derecha", "오른쪽 위", "右上", "右上", "Справа вверху", "Em cima à direita", "บนขวา"),
    "Boss part: Lower left": ("Lower left", "左下", "En bas à gauche", "In basso a sinistra", "Unten links", "Abajo a la izquierda", "왼쪽 아래", "左下", "左下", "Слева внизу", "Embaixo à esquerda", "ล่างซ้าย"),
}


def main():
    original = CATALOG.read_text(encoding="utf-8")
    data = json.loads(original)
    changed = False
    for key, values in ROWS.items():
        assert len(values) == len(LOCALES)
        assert all("{0}" not in value for value in values)
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
