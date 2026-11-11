"""Original FF1 slab-guide text, all twelve supported languages."""
import json,sys
from pathlib import Path
from modtext_opera_guide import LOCALES
P=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
ROWS={
'Speak to the elder to begin the trial.':('Speak to the elder to begin the trial.','長老に話しかけ、試練を始めてください。',"Parlez à l'ancien pour commencer l'épreuve.",'Parla con l’anziano per iniziare la prova.','Sprich mit dem Ältesten, um die Prüfung zu beginnen.','Habla con el anciano para comenzar la prueba.','장로에게 말을 걸어 시련을 시작하세요.','與長老交談以開始試煉。','与长老交谈以开始试炼。','Поговорите со старейшиной, чтобы начать испытание.','Fale com o ancião para começar a provação.','พูดคุยกับผู้อาวุโสเพื่อเริ่มการทดสอบ'),
'Defeat the guardian.':('Defeat the guardian.','守護者を倒してください。','Battez le gardien.','Sconfiggi il guardiano.','Besiege den Wächter.','Derrota al guardián.','수호자를 쓰러뜨리세요.','擊敗守護者。','击败守护者。','Победите стража.','Derrote o guardião.','เอาชนะผู้พิทักษ์'),
'Open the chest after the battle.':('Open the chest after the battle.','戦闘後に宝箱を開けてください。','Ouvrez le coffre après le combat.','Apri il forziere dopo la battaglia.','Öffne nach dem Kampf die Truhe.','Abre el cofre después del combate.','전투 후 보물상자를 여세요.','戰鬥後打開寶箱。','战斗后打开宝箱。','После боя откройте сундук.','Abra o baú após a batalha.','เปิดหีบหลังการต่อสู้'),
'Speak to the giant.':('Speak to the giant.','巨人に話しかけてください。','Parlez au géant.','Parla con il gigante.','Sprich mit dem Riesen.','Habla con el gigante.','거인에게 말을 거세요.','與巨人交談。','与巨人交谈。','Поговорите с великаном.','Fale com o gigante.','พูดคุยกับยักษ์'),
'Speak to the sage to obtain the rod.':('Speak to the sage to obtain the rod.','賢者に話しかけ、杖を受け取ってください。','Parlez au sage pour obtenir le bâton.','Parla con il saggio per ottenere il bastone.','Sprich mit dem Weisen, um den Stab zu erhalten.','Habla con el sabio para obtener la vara.','현자에게 말을 걸어 지팡이를 받으세요.','與賢者交談以取得杖。','与贤者交谈以取得杖。','Поговорите с мудрецом, чтобы получить жезл.','Fale com o sábio para obter o cajado.','พูดคุยกับปราชญ์เพื่อรับคทา'),
'Examine the stone slab to use the rod.':('Examine the stone slab to use the rod.','石板を調べて杖を使ってください。','Examinez la dalle pour utiliser le bâton.','Esamina la lastra di pietra per usare il bastone.','Untersuche die Steinplatte, um den Stab zu benutzen.','Examina la losa de piedra para usar la vara.','석판을 조사하여 지팡이를 사용하세요.','調查石板以使用杖。','调查石板以使用杖。','Осмотрите каменную плиту, чтобы использовать жезл.','Examine a laje de pedra para usar o cajado.','สำรวจแผ่นหินเพื่อใช้คทา'),
'Examine the stone slab to play the Lute.':('Examine the stone slab to play the Lute.','石板を調べてリュートを弾いてください。','Examinez la dalle pour jouer du luth.','Esamina la lastra di pietra per suonare il liuto.','Untersuche die Steinplatte, um die Laute zu spielen.','Examina la losa de piedra para tocar el laúd.','석판을 조사하여 류트를 연주하세요.','調查石板以彈奏魯特琴。','调查石板以弹奏鲁特琴。','Осмотрите каменную плиту, чтобы сыграть на лютне.','Examine a laje de pedra para tocar o alaúde.','สำรวจแผ่นหินเพื่อเล่นพิณลูท'),
}
def main():
    original=P.read_text(encoding='utf-8');catalog=json.loads(original)
    for key,values in ROWS.items():assert len(values)==12 and values[0]==key and all(values)
    additions={key:dict(zip(LOCALES,values)) for key,values in ROWS.items() if key not in catalog}
    if '--check' in sys.argv:assert not additions,'Missing FF1 slab translations'
    if additions:
        insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]
        P.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8');catalog.update(additions)
    for key in ROWS:assert catalog[key]==dict(zip(LOCALES,ROWS[key]))
    print('FF1 guides: seven instructions in twelve languages;',len(additions),'added')
if __name__=='__main__':main()
