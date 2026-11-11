"""Native-direction UI/solution keys and source-localized canonical Gau name."""
from pathlib import Path
import csv,json,re,sys
ROOT=Path(__file__).resolve().parents[1];D=ROOT.parent/'tools/mapdump/ff6_master'
TEXT={
'en':('Bring {0} in the active party, then request guidance again.','Choose Right at the junction.','Junction: {0}.'),
'ja':('{0}を現在のパーティに入れて、もう一度ガイドを呼び出してください。','分岐で右を選んでください。','分岐：{0}。'),
'fr':('Ajoutez {0} au groupe actif, puis demandez à nouveau le guide.','Choisissez la droite à la bifurcation.','Bifurcation : {0}.'),
'it':('Metti {0} nel gruppo attivo, poi richiedi di nuovo la guida.','Scegli Destra al bivio.','Bivio: {0}.'),
'de':('Nimm {0} in die aktive Gruppe auf und rufe die Hilfe erneut auf.','Wähle an der Abzweigung Rechts.','Abzweigung: {0}.'),
'es':('Añade a {0} al grupo activo y vuelve a pedir la guía.','Elige Derecha en la bifurcación.','Bifurcación: {0}.'),
'ko':('{0}를 현재 파티에 넣은 뒤 안내를 다시 요청하세요.','갈림길에서 오른쪽을 선택하세요.','갈림길: {0}.'),
'zht':('將{0}加入目前隊伍後，再次請求引導。','在岔路選擇右邊。','岔路：{0}。'),
'zhc':('将{0}加入当前队伍后，再次请求引导。','在岔路选择右边。','岔路：{0}。'),
'ru':('Добавьте {0} в активную группу, затем снова запросите подсказку.','На развилке выберите вправо.','Развилка: {0}.'),
'pt':('Coloque {0} no grupo ativo e peça orientação novamente.','Escolha Direita na bifurcação.','Bifurcação: {0}.'),
'th':('เพิ่ม{0}ในปาร์ตี้ปัจจุบัน แล้วขอคำแนะนำอีกครั้ง','เลือกขวาที่ทางแยก','ทางแยก: {0}'),
}
RETURN={
'en':'Return after the battle at Narshe.',
'ja':'ナルシェでの戦いの後に戻ってください。',
'fr':'Revenez après la bataille de Narshe.',
'it':'Torna dopo la battaglia di Narshe.',
'de':'Kehre nach der Schlacht von Narshe zurück.',
'es':'Vuelve después de la batalla de Narshe.',
'ko':'나르셰 전투 후에 돌아오세요.',
'zht':'納爾謝之戰後再回來。',
'zhc':'纳尔谢之战后再回来。',
'ru':'Вернитесь после битвы в Нарше.',
'pt':'Volte após a batalha de Narshe.',
'th':'กลับมาหลังการต่อสู้ที่นาร์เช',
}
def main():
 defaults=list(csv.DictReader((D/'character_default_name.txt').open(encoding='utf-8-sig')));row=next(v for v in defaults if v['character_id']=='12');assert row['tag_name']=='GAU';name_key=row['mes_id_name']
 names={}
 for lang in TEXT:
  source=dict(line.rstrip('\r\n').split('\t',1)for line in(D/('system_'+lang+'.txt')).read_text(encoding='utf-8-sig').splitlines()if'\t'in line)
  names[lang]=source[name_key]
 keys=('Bring Gau in the active party, then request guidance again.',*TEXT['en'][1:])
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8-sig'))
 for i,key in enumerate(keys):
  values={lang:(v[i].format(names[lang])if i==0 else v[i])for lang,v in TEXT.items()}
  assert len(values)==12 and all(set(re.findall(r'\{\d+\}',s))==set(re.findall(r'\{\d+\}',key))for s in values.values())
  if '--check'in sys.argv:assert data.get(key)==values,key
  else:
   assert key not in data or data[key]==values,key
   data[key]=values
 if '--check'in sys.argv:assert data.get(RETURN['en'])==RETURN
 else:
  assert RETURN['en']not in data or data[RETURN['en']]==RETURN
  data[RETURN['en']]=RETURN
  p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Crescent/Trench:four keys/all12 languages;canonical Gau name from actual game tables.')
if __name__=='__main__':main()
