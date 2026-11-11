"""Four original instructions in all twelve supported UI languages."""
from pathlib import Path
import json,sys
from author_solitary_guide import CATCH,FEED,REFRESH,RAFT
ROOT=Path(__file__).resolve().parents[1]
TEXT={
'en':('Catch this fish, then return to Cid.','Give Cid every fish you are carrying.','Talk to Cid to refresh the fish.','Board the raft.'),
'ja':('この魚を捕まえてシドのところへ戻ってください。','持っている魚をすべてシドに渡してください。','シドと話して出現する魚を変えてください。','いかだに乗ってください。'),
'fr':('Attrapez ce poisson, puis retournez voir Cid.','Donnez à Cid tous les poissons que vous transportez.','Parlez à Cid pour renouveler les poissons.','Montez sur le radeau.'),
'it':('Cattura questo pesce, poi torna da Cid.','Dai a Cid tutti i pesci che porti con te.','Parla con Cid per far comparire altri pesci.','Sali sulla zattera.'),
'de':('Fange diesen Fisch und kehre zu Cid zurück.','Gib Cid alle Fische, die du bei dir hast.','Sprich mit Cid, damit neue Fische erscheinen.','Steige auf das Floß.'),
'es':('Atrapa este pez y vuelve con Cid.','Dale a Cid todos los peces que llevas.','Habla con Cid para renovar los peces.','Sube a la balsa.'),
'ko':('이 물고기를 잡은 뒤 시드에게 돌아가세요.','가지고 있는 물고기를 모두 시드에게 주세요.','시드와 대화하여 나타나는 물고기를 바꾸세요.','뗏목에 타세요.'),
'zht':('抓住這條魚，然後回到西德身邊。','將身上所有的魚交給西德。','與西德交談以更換出現的魚。','登上木筏。'),
'zhc':('抓住这条鱼，然后回到西德身边。','将身上所有的鱼交给西德。','与西德交谈以更换出现的鱼。','登上木筏。'),
'ru':('Поймайте эту рыбу и вернитесь к Сиду.','Отдайте Сиду всю рыбу, которую несёте.','Поговорите с Сидом, чтобы появилась другая рыба.','Садитесь на плот.'),
'pt':('Pegue este peixe e volte para Cid.','Dê a Cid todos os peixes que você carrega.','Fale com Cid para renovar os peixes.','Suba na jangada.'),
'th':('จับปลาตัวนี้แล้วกลับไปหาซิด','มอบปลาทั้งหมดที่ถืออยู่ให้ซิด','คุยกับซิดเพื่อเปลี่ยนปลาที่ปรากฏ','ขึ้นแพ'),
}
if __name__=='__main__':
 assert len(TEXT)==12 and TEXT['en']==(CATCH,FEED,REFRESH,RAFT)
 p=ROOT/'data/ff6/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for i,key in enumerate(TEXT['en']):
  want={lang:values[i]for lang,values in TEXT.items()};assert all(want.values())
  if '--check'in sys.argv:assert data.get(key)==want
  else:assert key not in data or data[key]==want;data[key]=want
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Solitary:four instructions/all12 languages;no hidden numerical health or timed automation.')
