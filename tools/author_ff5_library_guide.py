"""Original guide decisions/localization only; never copy native programs."""
from pathlib import Path
import hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[1]
LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
PIN='1761ed14ac0938a38b21f8933c584c1811c0a40752361a988ca793afb26b548a'
VALUES={
 'Navigation data is loading':[
  'Navigation data is loading','経路案内データを読み込んでいます','Chargement des données de navigation',
  'Caricamento dei dati di navigazione','Navigationsdaten werden geladen','Cargando datos de navegación',
  '경로 안내 데이터를 불러오는 중입니다','正在載入導航資料','正在加载导航数据','Загружаются данные навигации',
  'กำลังโหลดข้อมูลนำทาง','Carregando dados de navegação'],
 'Follow the route to move the shelves, then wait.':[
  'Follow the route to move the shelves, then wait.','案内された経路を進んで本棚を動かし、止まるまで待ってください。',
  "Suivez l’itinéraire pour déplacer les étagères, puis attendez.",'Segui il percorso per spostare gli scaffali, poi aspetta.',
  'Folge der Route, um die Bücherregale zu bewegen, und warte dann.','Sigue la ruta para mover las estanterías y espera.',
  '안내 경로를 따라 책장을 움직인 뒤 기다리세요.','沿著路線移動書架，然後等待。','沿着路线移动书架，然后等待。',
  'Следуйте по маршруту, чтобы сдвинуть книжные стеллажи, затем подождите.','เดินตามเส้นทางเพื่อเลื่อนชั้นหนังสือ แล้วรอ','Siga a rota para mover as estantes e espere.'],
 'Examine the bookcase to move the shelves.':[
  'Examine the bookcase to move the shelves.','本棚を調べて動かしてください。','Examinez la bibliothèque pour déplacer les étagères.',
  'Esamina la libreria per spostare gli scaffali.','Untersuche das Bücherregal, um die Regale zu bewegen.',
  'Examina la estantería para mover los estantes.','책장을 조사해 움직이세요.','調查書架，使書架移動。','调查书架，使书架移动。',
  'Осмотрите книжный шкаф, чтобы сдвинуть стеллажи.','สำรวจตู้หนังสือเพื่อเลื่อนชั้นหนังสือ','Examine a estante para mover as prateleiras.'],
 'Approach the blocked bookcase to clear the passage.':[
  'Approach the blocked bookcase to clear the passage.','通路を塞ぐ本棚に近づいて、道を開けてください。',
  'Approchez de la bibliothèque qui bloque le chemin pour libérer le passage.','Avvicinati alla libreria che blocca il passaggio per liberarlo.',
  'Nähere dich dem Bücherregal, das den Weg versperrt, um den Durchgang freizumachen.',
  'Acércate a la estantería que bloquea el paso para despejarlo.','통로를 막는 책장에 다가가 길을 여세요.',
  '靠近擋路的書架，打通通道。','靠近挡路的书架，打通通道。',
  'Подойдите к преграждающему путь книжному шкафу, чтобы освободить проход.',
  'เข้าใกล้ตู้หนังสือที่ขวางทางเพื่อเปิดทาง','Aproxime-se da estante que bloqueia o caminho para liberar a passagem.'],
 'Examine the book and win the battle.':[
  'Examine the book and win the battle.','本を調べて戦闘に勝ってください。','Examinez le livre et remportez le combat.',
  'Esamina il libro e vinci la battaglia.','Untersuche das Buch und gewinne den Kampf.','Examina el libro y gana la batalla.',
  '책을 조사하고 전투에서 승리하세요.','調查書本，並贏得戰鬥。','调查书本，并赢得战斗。',
  'Осмотрите книгу и победите в бою.','สำรวจหนังสือแล้วชนะการต่อสู้','Examine o livro e vença a batalha.'],
 'Follow the route and win the battle.':[
  'Follow the route and win the battle.','案内された経路を進み、戦闘に勝ってください。','Suivez l’itinéraire et remportez le combat.',
  'Segui il percorso e vinci la battaglia.','Folge der Route und gewinne den Kampf.','Sigue la ruta y gana la batalla.',
  '안내 경로를 따라가 전투에서 승리하세요.','沿著路線前進，並贏得戰鬥。','沿着路线前进，并赢得战斗。',
  'Следуйте по маршруту и победите в бою.','เดินตามเส้นทางแล้วชนะการต่อสู้','Siga a rota e vença a batalha.'],
 'Move off this spot, then repeat the guide.':[
  'Move off this spot, then repeat the guide.','この場所から離れて、ガイドをもう一度実行してください。',
  'Quittez cet emplacement, puis répétez le guide.','Spostati da questo punto, poi ripeti la guida.',
  'Verlasse diese Stelle und rufe die Anleitung erneut auf.','Sal de este punto y repite la guía.',
  '이 지점에서 벗어난 뒤 안내를 다시 실행하세요.','離開這個位置，然後再次啟動引導。','离开这个位置，然后再次启动引导。',
  'Сойдите с этого места и снова запросите подсказку.','เดินออกจากจุดนี้ แล้วขอคำแนะนำอีกครั้ง','Saia deste ponto e repita o guia.']}

def main():
 path=ROOT/'data/ff5/puzzles.json';original=json.loads(path.read_text(encoding='utf-8-sig'))
 source=ROOT/'build_check/ff5_library_policy.json'
 policy=json.loads(source.read_text())if source.is_file()else original['library_policy']
 assert hashlib.sha256(json.dumps(policy,sort_keys=True,separators=(',',':'),ensure_ascii=False).encode()).hexdigest()==PIN
 assert policy['version']==1 and len(policy['rows'])==188
 assert len({row['key']for row in policy['rows']})==188
 for row in policy['rows']:
  m,f,h=map(int,row['key'].split(':'));assert 275<=m<=283 and 0<=f<256 and 0<=h<256 and 0<len(row['targets'])<=32
  for target in row['targets']:assert len(target)==5 and target[0]>=0 and 0<=target[1]<=2 and min(target[2:4])>=0 and target[4]in(0,1)
 updated=dict(original);updated['library_policy']=policy
 text_path=ROOT/'data/ff5/mod_text.json';catalog=json.loads(text_path.read_text(encoding='utf-8-sig'));expected=dict(catalog)
 expected.pop('Approach the blocked bookcase so {0} can clear the passage.',None)
 assert set(json.loads((ROOT/'data/common/mod_text.json').read_text(encoding='utf-8-sig'))['Exit to {0}'])==set(LANGS)
 for key,values in VALUES.items():
  assert len(values)==12
  row=dict(zip(LANGS,values));assert all(set(re.findall(r'\{\d+\}',v))==set(re.findall(r'\{\d+\}',key))for v in row.values())
  expected[key]=row
 if '--check'in sys.argv:
  assert original==updated,'Library policy resource differs from the qualified original decisions'
  assert catalog==expected,'Library instructions/locales differ'
 else:
  path.write_text(json.dumps(updated,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
  text_path.write_text(json.dumps(expected,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('FFV Library:188 original bounded policy observations, six instructions plus readiness message/12 languages, original engine-room recipe preserved; no native program embedded.')
if __name__=='__main__':main()
