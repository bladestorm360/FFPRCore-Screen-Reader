"""One mixed-operation instruction, using actual localized character/command names."""
from pathlib import Path
import csv,json,re,sys
ROOT=Path(__file__).resolve().parents[1];D=ROOT.parent/'tools/mapdump/ff6_master'
KEY='Fight {0}. When Relm joins, use Sketch on the enemy.'
TEXT={
'en':'Fight {0}. When {relm} joins, use {sketch} on the enemy.',
'ja':'{0}と戦ってください。{relm}が加わったら、敵に{sketch}を使ってください。',
'fr':'Combattez {0}. Quand {relm} vous rejoint, utilisez {sketch} sur l’ennemi.',
'it':'Combatti {0}. Quando arriva {relm}, usa {sketch} sul nemico.',
'de':'Bekämpfe {0}. Sobald {relm} dazukommt, benutze {sketch} gegen den Gegner.',
'es':'Combate contra {0}. Cuando se una {relm}, usa {sketch} contra el enemigo.',
'ko':'{0}와 싸우세요. {relm}이 합류하면 적에게 {sketch}를 사용하세요.',
'zht':'與{0}戰鬥。{relm}加入後，對敵人使用{sketch}。',
'zhc':'与{0}战斗。{relm}加入后，对敌人使用{sketch}。',
'ru':'Сразитесь с {0}. Когда присоединится {relm}, примените {sketch} к врагу.',
'pt':'Enfrente {0}. Quando {relm} se juntar ao grupo, use {sketch} no inimigo.',
'th':'ต่อสู้กับ{0} เมื่อ{relm}เข้าร่วม ให้ใช้{sketch}กับศัตรู',
}
def main():
 actor=next(r for r in csv.DictReader((D/'character_default_name.txt').open(encoding='utf-8-sig'))if r['character_id']=='9');assert actor['tag_name']=='RELM'
 values={}
 for lang,template in TEXT.items():
  system=dict(line.split('\t',1)for line in(D/('system_'+lang+'.txt')).read_text(encoding='utf-8-sig').splitlines()if'\t'in line)
  values[lang]=template.replace('{relm}',system[actor['mes_id_name']]).replace('{sketch}',system['MSG_CMD_NAME_14'])
 assert values['en']==KEY and all(set(re.findall(r'\{\d+\}',s))=={'{0}'}for s in values.values())
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 if '--check'in sys.argv:assert data.get(KEY)==values
 else:
  assert KEY not in data or data[KEY]==values;data[KEY]=values;p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Esper:one Sketch solution/all12 locales;actual canonical Relm and rendered command names.')
if __name__=='__main__':main()
