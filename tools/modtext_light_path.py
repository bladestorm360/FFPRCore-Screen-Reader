"""Original, concise Narshe guide instructions in every supported mod language."""
import json
import sys
from pathlib import Path
PATH=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
LOCALES=('en','ja','fr','it','de','es','ko','zht','zhc','ru','pt','th')
ROWS={
'Start the light demonstration.':(
 'Start the light demonstration.','光の道の実演を始めてください。','Lancez la démonstration du chemin lumineux.','Avvia la dimostrazione del percorso luminoso.','Starte die Vorführung des Lichtpfads.','Inicia la demostración del camino de luz.','빛의 길 시범을 시작하세요.','開始光路示範。','开始光路演示。','Начните показ светового пути.','Inicie a demonstração do caminho de luz.','เริ่มการสาธิตเส้นทางแสง'),
'Follow the light path.':(
 'Follow the light path.','光の道を進んでください。','Suivez le chemin lumineux.','Segui il percorso luminoso.','Folge dem Lichtpfad.','Sigue el camino de luz.','빛의 길을 따라가세요.','沿著光路前進。','沿着光路前进。','Следуйте по световому пути.','Siga o caminho de luz.','เดินตามเส้นทางแสง'),
'Leave through the northern exit.':(
 'Leave through the northern exit.','北の出口から出てください。','Sortez par la sortie nord.','Esci dall’uscita a nord.','Verlasse den Raum durch den Nordausgang.','Sal por la salida norte.','북쪽 출구로 나가세요.','從北方出口離開。','从北方出口离开。','Выйдите через северный выход.','Saia pela saída ao norte.','ออกทางทางออกด้านเหนือ'),
'Press a direction to resolve the light trap, then use Shift+G.':(
 'Press a direction to resolve the light trap, then use Shift+G.','方向を押して光の罠を処理し、その後Shift+Gを押してください。','Appuyez sur une direction pour résoudre le piège lumineux, puis utilisez Shift+G.','Premi una direzione per risolvere la trappola luminosa, poi usa Shift+G.','Drücke eine Richtung, um die Lichtfalle aufzulösen, dann Shift+G.','Pulsa una dirección para resolver la trampa de luz y luego usa Shift+G.','방향을 눌러 빛 함정을 해결한 다음 Shift+G를 누르세요.','按方向鍵處理光之陷阱，然後按Shift+G。','按方向键处理光之陷阱，然后按Shift+G。','Нажмите направление, чтобы разрешить световую ловушку, затем Shift+G.','Pressione uma direção para resolver a armadilha de luz e use Shift+G.','กดทิศทางเพื่อจัดการกับดักแสง แล้วกด Shift+G'),
'Leave and re-enter this room to restart the light path.':(
 'Leave and re-enter this room to restart the light path.','部屋を出て入り直すと光の道をやり直せます。','Sortez de cette salle et revenez pour recommencer le chemin lumineux.','Esci e rientra nella stanza per ricominciare il percorso luminoso.','Verlasse diesen Raum und betrete ihn erneut, um den Lichtpfad neu zu beginnen.','Sal de esta sala y vuelve a entrar para reiniciar el camino de luz.','방을 나갔다 다시 들어와 빛의 길을 재시작하세요.','離開再進入此房間，重新開始光路。','离开再进入此房间，重新开始光路。','Выйдите из комнаты и вернитесь, чтобы начать световой путь заново.','Saia e entre novamente nesta sala para reiniciar o caminho de luz.','ออกจากห้องแล้วเข้ามาใหม่เพื่อเริ่มเส้นทางแสงอีกครั้ง'),
}
def main():
 original=PATH.read_text(encoding='utf-8');catalog=json.loads(original)
 additions={key:dict(zip(LOCALES,values)) for key,values in ROWS.items() if key not in catalog}
 for key,values in ROWS.items():
  assert len(values)==12 and values[0]==key and all(values)
 if '--check' in sys.argv:assert not additions,'Missing light-puzzle translations; author them before building'
 if additions:
  insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]
  PATH.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8')
  catalog.update(additions)
 for key in ROWS:assert set(catalog[key])==set(LOCALES) and all(catalog[key].values())
 print(f'Light path: {len(ROWS)} instructions verified in12 languages; {len(additions)} added')
if __name__=='__main__':main()
