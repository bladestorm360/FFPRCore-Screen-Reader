"""Original battle-party control labels in the twelve supported UI languages."""
import json,sys
from pathlib import Path
from modtext_opera_guide import LOCALES
ROOT=Path(__file__).resolve().parents[1];P=ROOT/'data/common/mod_text.json'
SCOPE=('Battle party','戦闘パーティー','Groupe en combat','Gruppo in battaglia','Kampfgruppe','Grupo en combate','전투 파티','戰鬥隊伍','战斗队伍','Группа в бою','Grupo em batalha','ปาร์ตี้ในการต่อสู้')
LABELS=('Battle: party slot {0} statistics','戦闘：パーティ枠{0}のステータス','Combat : statistiques du membre {0}','Battaglia: statistiche del membro {0}','Kampf: Werte von Gruppenplatz {0}','Combate: estadísticas del puesto {0}','전투: 파티 슬롯 {0} 능력치','戰鬥：隊伍欄位{0}的狀態','战斗：队伍栏位{0}的状态','Бой: характеристики участника {0}','Batalha: atributos do membro {0}','การต่อสู้: สถานะสมาชิกช่อง {0}')
def main():
 original=P.read_text(encoding='utf-8');catalog=json.loads(original)
 rows={SCOPE[0]:dict(zip(LOCALES,SCOPE))}
 for slot in range(1,6):rows[LABELS[0].format(slot)]={locale:pattern.format(slot)for locale,pattern in zip(LOCALES,LABELS)}
 assert len(SCOPE)==len(LABELS)==12
 additions={key:values for key,values in rows.items()if key not in catalog}
 if '--check'in sys.argv:assert not additions,'Missing battle-party labels'
 elif additions:
  insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]
  P.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8');catalog.update(additions)
 assert all(catalog[key]==values for key,values in rows.items())
 print('Battle-party scope and five slot labels: six keys in twelve languages;',len(additions),'added')
if __name__=='__main__':main()
