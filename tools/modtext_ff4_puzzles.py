"""Original FFIV guide instructions and exact official terms in12 languages."""
from pathlib import Path
import hashlib,json,re,sys
from modtext_opera_guide import LOCALES
ROOT=Path(__file__).resolve().parents[1];P=ROOT/'data/common/mod_text.json'
ROWS={
'Obtain {0} from your ally.':('Obtain {0} from your ally.','仲間から{0}を受け取ってください。','Obtenez {0} auprès de votre allié.','Ottieni {0} dal tuo alleato.','Hole {0} von deinem Verbündeten.','Consigue {0} de tu aliado.','동료에게서 {0}을 받으세요.','向同伴取得{0}。','向同伴取得{0}。','Получите {0} у союзника.','Obtenha {0} com seu aliado.','รับ {0} จากเพื่อนร่วมทีม'),
'Remove metal equipment before exploring. Fight the first encounter, then wait for the harp scene.':('Remove metal equipment before exploring. Fight the first encounter, then wait for the harp scene.','探索前に金属装備を外してください。最初の戦闘を行い、竪琴の場面が終わるまで待ってください。',"Retirez l’équipement métallique avant d’explorer. Livrez le premier combat, puis attendez la scène de la harpe.",'Rimuovi l’equipaggiamento metallico prima di esplorare. Affronta il primo scontro, poi aspetta la scena dell’arpa.','Lege vor der Erkundung Metallausrüstung ab. Bestreite den ersten Kampf und warte dann auf die Harfenszene.','Quítate el equipo metálico antes de explorar. Libra el primer combate y espera la escena del arpa.','탐험 전에 금속 장비를 해제하세요. 첫 전투를 치른 뒤 하프 장면을 기다리세요.','探索前卸下金屬裝備。進行第一次戰鬥，然後等待豎琴事件結束。','探索前卸下金属装备。进行第一次战斗，然后等待竖琴事件结束。','Перед исследованием снимите металлическое снаряжение. Пройдите первый бой и дождитесь сцены с арфой.','Remova o equipamento metálico antes de explorar. Enfrente o primeiro combate e espere a cena da harpa.','ถอดอุปกรณ์โลหะก่อนสำรวจ ต่อสู้ครั้งแรกแล้วรอฉากพิณ'),
'Equip metal equipment again, then defeat the guardian.':('Equip metal equipment again, then defeat the guardian.','金属装備を再び装備し、守護者を倒してください。','Remettez l’équipement métallique, puis battez le gardien.','Indossa di nuovo l’equipaggiamento metallico, poi sconfiggi il guardiano.','Lege die Metallausrüstung wieder an und besiege den Wächter.','Vuelve a ponerte el equipo metálico y derrota al guardián.','금속 장비를 다시 착용한 뒤 수호자를 처치하세요.','重新穿上金屬裝備，然後擊敗守衛。','重新穿上金属装备，然后击败守卫。','Снова наденьте металлическое снаряжение и победите стража.','Equipe novamente o equipamento metálico e derrote o guardião.','สวมอุปกรณ์โลหะอีกครั้งแล้วปราบผู้พิทักษ์'),
'Use {0} at the well, then wait for the scene to finish.':('Use {0} at the well, then wait for the scene to finish.','井戸で{0}を使い、イベントが終わるまで待ってください。',"Utilisez {0} au puits, puis attendez la fin de la scène.",'Usa {0} al pozzo, poi aspetta la fine della scena.','Benutze {0} am Brunnen und warte, bis die Szene endet.','Usa {0} en el pozo y espera a que termine la escena.','우물에서 {0}을 사용한 뒤 장면이 끝날 때까지 기다리세요.','在井邊使用{0}，然後等待事件結束。','在井边使用{0}，然后等待事件结束。','Используйте {0} у колодца и дождитесь конца сцены.','Use {0} no poço e espere a cena terminar.','ใช้ {0} ที่บ่อน้ำแล้วรอจนฉากจบ')}
TERMS={'FF4 item Baron Key':'MSG_KEY_NAME_03','FF4 item Twin Harp':'MSG_KEY_NAME_08',
'FF4 item Magma Rock':'MSG_KEY_NAME_06','FF4 item Luca Necklace':'MSG_KEY_NAME_07','FF4 item Dr Lugae Key':'MSG_KEY_NAME_14'}
def main():
 sys.path.insert(0,str(ROOT.parent/'tools'));import dump_map_objects as archive
 bundles=list(Path(archive.aa_path('ff4')).glob('message_assets_*.bundle'));assert len(bundles)==1
 _,assets=archive.load_texts(str(bundles[0]));rows={k:dict(zip(LOCALES,v)) for k,v in ROWS.items()}
 assert all(len(v)==12 and v[0]==k and all(v) for k,v in ROWS.items())
 for key,message in TERMS.items():
  rows[key]={}
  for locale in LOCALES:
   texts=assets['system_'+locale];assert len(texts)==1
   terms=dict(line.rstrip('\r\n').split('\t',1) for line in texts[0].splitlines() if '\t' in line)
   rows[key][locale]=re.sub(r'<[^>]*>','',terms[message]).strip();assert rows[key][locale]
 original=P.read_text(encoding='utf-8');catalog=json.loads(original);additions={k:v for k,v in rows.items() if k not in catalog}
 if '--check' in sys.argv:assert not additions,'Missing FFIV puzzle translations'
 if additions:
  insertion=json.dumps(additions,ensure_ascii=False,indent=2)[2:-2];P.write_text(original[:original.rfind('}')].rstrip()+',\n'+insertion+'\n}\n',encoding='utf-8');catalog.update(additions)
 for k,v in rows.items():assert catalog[k]==v,('FFIV translation differs',k)
 (ROOT/'build_check/ff4_puzzle_locales.json').write_text(json.dumps(dict(instructions=len(ROWS),terms=TERMS,locales=LOCALES,bundle=bundles[0].name,
  bundle_sha256=hashlib.sha256(bundles[0].read_bytes()).hexdigest()),indent=2)+'\n',encoding='utf-8')
 print('FFIV puzzles:',len(ROWS),'instructions/',len(TERMS),'exact terms in12 languages;',len(additions),'added.')
if __name__=='__main__':main()
