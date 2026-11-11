"""Original Surgate guide/resource author. Native bodies remain external."""
from pathlib import Path
import csv,json,sys
import bake_maps
ROOT=Path(__file__).resolve().parents[1];DUMP=ROOT.parent/'tools/mapdump'
LANGS=('en','ja','fr','it','de','es','ko','zht','zhc','ru','th','pt')
TEXT={
 'Pick up the book.':['Pick up the book.','本を拾ってください。','Ramassez le livre.','Raccogli il libro.','Hebe das Buch auf.','Recoge el libro.','책을 주우세요.','撿起書本。','捡起书本。','Поднимите книгу.','หยิบหนังสือ','Pegue o livro.'],
 'Put the book on this shelf and choose Yes.':['Put the book on this shelf and choose Yes.','この本棚に本を戻し、「はい」を選んでください。','Rangez le livre sur cette étagère et choisissez Oui.','Metti il libro su questo scaffale e scegli Sì.','Stelle das Buch in dieses Regal und wähle Ja.','Pon el libro en esta estantería y elige Sí.','이 책장에 책을 놓고 예를 선택하세요.','將書放入這個書架，並選擇「是」。','将书放入这个书架，并选择“是”。','Поставьте книгу на эту полку и выберите «Да».','วางหนังสือบนชั้นนี้แล้วเลือกใช่','Coloque o livro nesta estante e escolha Sim.'],
 'Talk to the librarian.':['Talk to the librarian.','司書に話しかけてください。','Parlez à la bibliothécaire.','Parla con la bibliotecaria.','Sprich mit der Bibliothekarin.','Habla con la bibliotecaria.','사서에게 말을 거세요.','與圖書管理員交談。','与图书管理员交谈。','Поговорите с библиотекарем.','คุยกับบรรณารักษ์','Fale com a bibliotecária.'],
 'Leave the library and enter again to restart the unfinished arrangement.':['Leave the library and enter again to restart the unfinished arrangement.','図書室を出て入り直し、途中の本の整理を最初からやり直してください。','Sortez de la bibliothèque et revenez pour recommencer le rangement inachevé.','Esci dalla biblioteca e rientra per ricominciare la sistemazione incompleta.','Verlasse die Bibliothek und betrete sie erneut, um die unvollständige Sortierung neu zu beginnen.','Sal de la biblioteca y vuelve a entrar para reiniciar la colocación sin terminar.','도서관을 나갔다 다시 들어와 미완성된 책 정리를 처음부터 시작하세요.','離開圖書室後再次進入，重新開始尚未完成的書本整理。','离开图书室后再次进入，重新开始尚未完成的书本整理。','Выйдите из библиотеки и войдите снова, чтобы заново начать незавершённую расстановку.','ออกจากห้องสมุดแล้วกลับเข้ามาเพื่อเริ่มจัดหนังสือที่ยังไม่เสร็จใหม่','Saia da biblioteca e entre novamente para reiniciar a organização incompleta.'],
 'Return to the library.':['Return to the library.','図書室に戻ってください。','Retournez à la bibliothèque.','Torna nella biblioteca.','Kehre in die Bibliothek zurück.','Vuelve a la biblioteca.','도서관으로 돌아가세요.','返回圖書室。','返回图书室。','Вернитесь в библиотеку.','กลับไปที่ห้องสมุด','Volte à biblioteca.']}
def main():
 objects=json.loads((ROOT.parent/'tools/ff5_surgate_20261102/objects.json').read_text(encoding='utf-8'))
 maps={int(r['id']):r['map_name']for r in csv.DictReader((DUMP/'ff5_master/map.txt').open(encoding='utf-8-sig'))};info=json.loads((DUMP/'ff5_submaps.json').read_text(encoding='utf-8'))
 room=maps[160];rows=[r for r in objects if r['submap']==room];ox,oy,_,_=bake_maps.map_bounds(info[room],rows)
 def actor(entity,kind,text):
  selected=[r for r in rows if r['type_name']==kind and r['props'].get('entity_id')==entity];assert len(selected)==1
  o=selected[0];return dict(map=160,entity=entity,raw=o['name'],layer=bake_maps.layer_index(o['layer']),x=o['tx']-ox,y=bake_maps.tile_row(o)-oy,text=text)
 books=[]
 for i,(book,shelves)in enumerate(zip((17,18,19),((21,35,36),(22,50),(23,51,52)))):
  books.append(dict(placed_flag=521+i,held_flag=524+i,pickup=actor(book,'Entity','Pick up the book.'),shelves=[actor(id,'Entity','Put the book on this shelf and choose Yes.')for id in shelves]))
 guide=dict(id='surgate_books',maps=[159,160],books=books,librarian=actor(20,'NPC','Talk to the librarian.'),reset_exit=actor(1,'GotoMap','Leave the library and enter again to restart the unfinished arrangement.'),return_point=dict(map=160,layer=1,x=12,y=16,text='Return to the library.'))
 path=ROOT/'data/ff5/puzzles.json';data=json.loads(path.read_text(encoding='utf-8'));expected=dict(data);expected['book_guide']=guide
 textpath=ROOT/'data/ff5/mod_text.json';catalog=json.loads(textpath.read_text(encoding='utf-8'));translated=dict(catalog)
 for key,values in TEXT.items():assert len(values)==12 and values[0]==key;translated[key]=dict(zip(LANGS,values))
 if '--check'in sys.argv:assert data==expected and catalog==translated,'Surgate resources/locales differ'
 else:path.write_text(json.dumps(expected,ensure_ascii=False,indent=2)+'\n',encoding='utf-8');textpath.write_text(json.dumps(translated,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
 print('Surgate resources: three books/eight correct shelf contacts/librarian/native reset exit; five instructions in12 languages; other resources preserved.')
if __name__=='__main__':main()
