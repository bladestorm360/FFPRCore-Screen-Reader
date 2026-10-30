"""Maintain the native FFIII forced EXP-tally acknowledgement in all locales."""
from pathlib import Path
import json

ROOT = Path(__file__).resolve().parents[1]
VALUES = {
    'en': 'EXP tally complete', 'ja': '経験値の集計完了',
    'fr': 'Décompte des EXP terminé', 'it': 'Conteggio EXP completato',
    'de': 'EP-Zählung abgeschlossen', 'es': 'Recuento de EXP completado',
    'ko': '경험치 집계 완료', 'zht': '經驗值結算完成', 'zhc': '经验值结算完成',
    'ru': 'Подсчёт опыта завершён', 'th': 'นับค่า EXP เสร็จสิ้น',
    'pt': 'Contagem de EXP concluída',
}

if __name__ == '__main__':
    path = ROOT / 'data/common/mod_text.json'
    catalog = json.loads(path.read_text(encoding='utf-8'))
    assert set(VALUES) == set(catalog['Exit to {0}'])
    assert all(VALUES.values())
    catalog['EXP tally complete'] = VALUES
    path.write_text(json.dumps(catalog, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')
    print('EXP tally completion: 1 key, all 12 supported locales')
