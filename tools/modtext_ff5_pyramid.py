"""Original short Pyramid instructions in every supported game language."""
from pathlib import Path
import json,sys
from author_ff5_pyramid import SWITCH,TRIGGER,GO,WAIT,TABLET
R=Path(__file__).resolve().parents[1]
TEXT={
'en':(SWITCH,TRIGGER,GO,WAIT,TABLET,'Defeat the guardians.'),
'ja':('スイッチを操作してください。','この地点を通過して、もう一度ガイドを呼び出してください。','今です。安全な床まで進んでください。','床が切り替わるまで待ってください。','石板に触れ、イベントが終わるまで待ってください。','守護者たちを倒してください。'),
'fr':('Actionnez l’interrupteur.','Traversez ce point, puis redemandez le guidage.','Allez-y. Rejoignez le sol sûr.','Attendez que le sol change.','Touchez la tablette et attendez la fin des scènes.','Battez les gardiens.'),
'it':('Aziona l’interruttore.','Attraversa questo punto, poi richiedi la guida.','Vai ora. Raggiungi il pavimento sicuro.','Attendi che il pavimento cambi.','Tocca la tavoletta e attendi la fine delle scene.','Sconfiggi i guardiani.'),
'de':('Betätige den Schalter.','Überquere diesen Punkt und fordere erneut Führung an.','Jetzt los. Gehe zum sicheren Boden.','Warte, bis sich der Boden ändert.','Berühre die Tafel und warte das Ende der Szenen ab.','Besiege die Wächter.'),
'es':('Acciona el interruptor.','Cruza este punto y vuelve a solicitar la guía.','Ve ahora. Llega al suelo seguro.','Espera a que cambie el suelo.','Toca la tablilla y espera a que terminen las escenas.','Derrota a los guardianes.'),
'ko':('스위치를 조작하세요.','이 지점을 지나고 안내를 다시 요청하세요.','지금 이동하세요. 안전한 바닥으로 가세요.','바닥이 바뀔 때까지 기다리세요.','석판을 만지고 장면이 끝날 때까지 기다리세요.','수호자들을 쓰러뜨리세요.'),
'zht':('操作開關。','通過此處，然後再次請求引導。','現在前進，到安全的地板。','等待地板切換。','觸碰石板，等待劇情結束。','打倒守護者。'),
'zhc':('操作开关。','通过此处，然后再次请求引导。','现在前进，到安全的地板。','等待地板切换。','触碰石板，等待剧情结束。','打倒守护者。'),
'ru':('Нажмите переключатель.','Пройдите через эту точку и снова запросите указания.','Идите сейчас. Дойдите до безопасного пола.','Дождитесь смены пола.','Коснитесь таблички и дождитесь окончания сцен.','Победите стражей.'),
'pt':('Acione o interruptor.','Atravesse este ponto e solicite a orientação novamente.','Vá agora. Alcance o piso seguro.','Espere o piso mudar.','Toque a tábua e espere as cenas terminarem.','Derrote os guardiões.'),
'th':('ใช้งานสวิตช์','ผ่านจุดนี้ แล้วขอคำแนะนำอีกครั้ง','ไปตอนนี้ มุ่งไปยังพื้นปลอดภัย','รอให้พื้นเปลี่ยน','แตะแผ่นศิลาแล้วรอจนฉากจบ','เอาชนะผู้พิทักษ์'),
}
ENEMY={'en':'Defeat the enemy.','ja':'敵を倒してください。','fr':'Battez l’ennemi.','it':'Sconfiggi il nemico.','de':'Besiege den Gegner.','es':'Derrota al enemigo.','ko':'적을 쓰러뜨리세요.','zht':'打倒敵人。','zhc':'打倒敌人。','ru':'Победите врага.','pt':'Derrote o inimigo.','th':'เอาชนะศัตรู'}
TEXT={lang:(*v,ENEMY[lang])for lang,v in TEXT.items()}
RESUME={'en':'Resume timers until the floor changes.','ja':'床が切り替わるまでタイマーを再開してください。','fr':'Relancez les minuteries jusqu’au changement du sol.','it':'Riattiva i timer finché il pavimento non cambia.','de':'Lasse die Timer weiterlaufen, bis sich der Boden ändert.','es':'Reanuda los temporizadores hasta que cambie el suelo.','ko':'바닥이 바뀔 때까지 타이머를 재개하세요.','zht':'恢復計時器，直到地板切換。','zhc':'恢复计时器，直到地板切换。','ru':'Возобновите таймеры до смены пола.','pt':'Retome os temporizadores até o piso mudar.','th':'เดินตัวจับเวลาต่อจนกว่าพื้นจะเปลี่ยน'}
TEXT={lang:(*v,RESUME[lang])for lang,v in TEXT.items()}
if __name__=='__main__':
 assert len(TEXT)==12 and all(len(v)==8 for v in TEXT.values());p=R/'data/ff5/mod_text.json';data=json.loads(p.read_text(encoding='utf-8'))
 for i,key in enumerate(TEXT['en']):
  wanted={lang:v[i]for lang,v in TEXT.items()};assert all(wanted.values())
  if '--check'in sys.argv:assert data.get(key)==wanted
  else:assert key not in data or data[key]==wanted;data[key]=wanted
 if '--check'not in sys.argv:p.write_text(json.dumps(data,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Pyramid:eight short instructions/all12 supported languages.')
