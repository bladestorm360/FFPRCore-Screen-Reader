"""Maintain the native Invincible manual crossing cue in all12 UI languages."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
VALUES={'en':'fly over the mountains {0}','ja':'{0}の山を越えて飛ぶ',
        'fr':'survolez les montagnes vers {0}','it':'sorvola le montagne verso {0}',
        'de':'über die Berge nach {0} fliegen','es':'vuela sobre las montañas hacia {0}',
        'ko':'{0}쪽 산을 넘어 비행','zht':'向{0}飛越山脈','zhc':'向{0}飞越山脉',
        'ru':'перелетите через горы: {0}','th':'บินข้ามภูเขาไปทาง{0}',
        'pt':'sobrevoe as montanhas na direção {0}'}
if __name__=='__main__':
    path=ROOT/'data/common/mod_text.json';catalog=json.loads(path.read_text(encoding='utf-8'))
    assert set(VALUES)==set(catalog['Can land'])
    catalog['fly over the mountains {0}']=VALUES
    path.write_text(json.dumps(catalog,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
    print('Invincible mountain guidance: all12 supported languages updated')
