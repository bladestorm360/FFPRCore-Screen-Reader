"""Original custom-audio controls; twelve-language, additive catalog author/check."""
import json,re,sys
from pathlib import Path
from modtext_opera_guide import LOCALES
P=Path(__file__).resolve().parents[1]/'data/common/mod_text.json'
ROWS={
'Custom sounds':('Custom sounds','カスタム効果音','Sons personnalisés','Suoni personalizzati','Eigene Klänge','Sonidos personalizados','사용자 지정 효과음','自訂音效','自定义音效','Свои звуки','Sons personalizados','เสียงที่กำหนดเอง'),
'Custom sound settings':('Custom sound settings','カスタム効果音の設定','Réglages des sons personnalisés','Impostazioni suoni personalizzati','Eigene Klänge einstellen','Ajustes de sonidos personalizados','사용자 지정 효과음 설정','自訂音效設定','自定义音效设置','Настройки своих звуков','Configurações dos sons personalizados','ตั้งค่าเสียงที่กำหนดเอง'),
'Use custom sound':('Use custom sound','カスタム効果音を使用','Utiliser le son personnalisé','Usa suono personalizzato','Eigenen Klang verwenden','Usar sonido personalizado','사용자 지정 효과음 사용','使用自訂音效','使用自定义音效','Использовать свой звук','Usar som personalizado','ใช้เสียงที่กำหนดเอง'),
'Custom sound volume':('Custom sound volume','カスタム効果音の音量','Volume du son personnalisé','Volume suono personalizzato','Lautstärke des eigenen Klangs','Volumen del sonido personalizado','사용자 지정 효과음 음량','自訂音效音量','自定义音效音量','Громкость своего звука','Volume do som personalizado','ระดับเสียงที่กำหนดเอง'),
'Loop custom wall sound':('Loop custom wall sound','カスタム壁音をループ','Boucler le son de mur personnalisé','Ripeti in ciclo il suono del muro','Eigenen Wandklang wiederholen','Repetir en bucle el sonido de pared','사용자 지정 벽 효과음 반복 재생','循環播放自訂牆壁音效','循环播放自定义墙壁音效','Зациклить свой звук стены','Repetir som de parede em ciclo','วนซ้ำเสียงผนังที่กำหนดเอง'),
'Repeat interval':('Repeat interval','繰り返し間隔','Intervalle de répétition','Intervallo di ripetizione','Wiederholungsabstand','Intervalo de repetición','반복 간격','重複間隔','重复间隔','Интервал повтора','Intervalo de repetição','ช่วงเวลาการเล่นซ้ำ'),
'Original timing':('Original timing','元の間隔','Rythme d’origine','Tempi originali','Ursprünglicher Abstand','Ritmo original','기존 간격','原有間隔','原有间隔','Исходный интервал','Ritmo original','จังหวะเดิม'),
'{0} ms':('{0} ms','{0}ミリ秒','{0} ms','{0} ms','{0} ms','{0} ms','{0}밀리초','{0}毫秒','{0}毫秒','{0} мс','{0} ms','{0} มิลลิวินาที'),
'Preview':('Preview','試聴','Écouter','Ascolta','Anhören','Escuchar','미리 듣기','試聽','试听','Прослушать','Ouvir','ฟังตัวอย่าง'),
'Custom sounds help':(
 'Use installed custom WAV sounds. Off restores generated sounds. Existing sound switches still apply.',
 '導入済みのカスタムWAV効果音を使用します。オフで標準の生成音に戻ります。既存の効果音設定も有効です。',
 'Utilise les sons WAV personnalisés installés. Désactivé : sons générés. Les autres réglages sonores restent actifs.',
 'Usa i WAV personalizzati installati. Disattivato ripristina i suoni generati. Restano validi gli altri interruttori audio.',
 'Installierte eigene WAV-Klänge verwenden. Aus stellt erzeugte Klänge wieder her. Bisherige Klangschalter gelten weiter.',
 'Usa los WAV personalizados instalados. Al desactivar, restaura los sonidos generados. Siguen vigentes los demás ajustes.',
 '설치된 사용자 지정 WAV를 사용합니다. 끄면 기본 생성음을 복원합니다. 기존 효과음 켜기 설정도 적용됩니다.',
 '使用已安裝的自訂WAV音效。關閉時還原為生成的預設音效。原有音效開關仍然有效。',
 '使用已安装的自定义WAV音效。关闭时恢复生成的默认音效。原有音效开关仍然有效。',
 'Использует установленные свои WAV. Выкл. возвращает сгенерированные звуки. Остальные переключатели звука сохраняют действие.',
 'Usa os WAV personalizados instalados. Desativado restaura os sons gerados. Os outros controles de som continuam válidos.',
 'ใช้ไฟล์ WAV ที่ติดตั้งไว้ เมื่อปิดจะกลับไปใช้เสียงเริ่มต้น สวิตช์เสียงเดิมยังมีผล'),
'Use custom sound help':(
 'Off restores this sound’s generated default. Missing or invalid WAV files also use the default.',
 'オフでこの効果音を標準の生成音に戻します。WAVファイルがない場合や無効な場合も標準音を使用します。',
 'Désactivé : son généré par défaut. Un WAV absent ou invalide utilise aussi le son par défaut.',
 'Disattivato ripristina il suono generato predefinito. Anche WAV mancanti o non validi usano il suono predefinito.',
 'Aus stellt diesen erzeugten Standardklang wieder her. Fehlende oder ungültige WAV-Dateien verwenden ebenfalls den Standard.',
 'Desactivar restaura el sonido generado predeterminado. Un WAV ausente o inválido también usa el predeterminado.',
 '끄면 이 효과음의 기본 생성음을 복원합니다. WAV가 없거나 유효하지 않아도 기본음을 사용합니다.',
 '關閉時還原此音效的生成預設音。WAV遺失或無效時也使用預設音。',
 '关闭时恢复此音效的生成默认音。WAV缺失或无效时也使用默认音。',
 'Выкл. возвращает исходный сгенерированный звук. При отсутствии или ошибке WAV также используется исходный звук.',
 'Desativado restaura o som gerado padrão. WAV ausente ou inválido também usa o padrão.',
 'เมื่อปิดจะใช้เสียงเริ่มต้นของเสียงนี้ หากไฟล์ WAV หายหรือใช้ไม่ได้จะใช้เสียงเริ่มต้นเช่นกัน'),
'Custom sound volume help':(
 'Adjust only this custom sound, zero to one hundred percent. Fifty keeps its original gain. Stereo and HRTF values are separate.',
 'このカスタム音のみを0～100パーセントで調整します。50が元の音量です。ステレオとHRTFの値は別々です。',
 'Règle ce son personnalisé de zéro à cent pour cent. Cinquante garde le gain initial. Valeurs stéréo et HRTF séparées.',
 'Regola solo questo suono da zero a cento per cento. Cinquanta mantiene il guadagno originale. Valori stereo e HRTF separati.',
 'Nur diesen eigenen Klang von null bis hundert Prozent regeln. Fünfzig behält die ursprüngliche Verstärkung. Stereo und HRTF sind getrennt.',
 'Ajusta solo este sonido de cero a cien por ciento. Cincuenta conserva la ganancia original. Estéreo y HRTF son independientes.',
 '이 사용자 지정 효과음만 0~100퍼센트로 조절합니다. 50은 원래 음량입니다. 스테레오와 HRTF 값은 별도입니다.',
 '僅調整此自訂音效，範圍0到100%。50保留原有增益。立體聲與HRTF分別儲存。',
 '仅调整此自定义音效，范围0到100%。50保留原有增益。立体声与HRTF分别保存。',
 'Громкость только этого своего звука от нуля до ста процентов. Пятьдесят сохраняет исходное усиление. Стерео и HRTF хранятся отдельно.',
 'Ajusta apenas este som de zero a cem por cento. Cinquenta mantém o ganho original. Estéreo e HRTF são separados.',
 'ปรับเฉพาะเสียงนี้จากศูนย์ถึงหนึ่งร้อยเปอร์เซ็นต์ ห้าสิบคือระดับเดิม ค่าสเตอริโอและ HRTF แยกกัน'),
'Loop custom wall sound help':(
 'On loops the custom wall sound continuously. Off plays timed pings. The WAV designer must provide a clean loop seam.',
 'オンでカスタム壁音を連続ループします。オフでは一定間隔で鳴らします。自然な継ぎ目はWAV制作者が用意してください。',
 'Activé : boucle continue du son de mur. Désactivé : signaux espacés. Le créateur du WAV doit assurer une boucle sans coupure.',
 'Attivo ripete continuamente il suono del muro. Disattivo emette segnali temporizzati. Il creatore del WAV deve curare il punto di ciclo.',
 'Ein wiederholt den eigenen Wandklang fortlaufend. Aus spielt einzelne Signale. Der WAV-Ersteller sorgt für einen nahtlosen Übergang.',
 'Activado repite el sonido de pared continuamente. Desactivado reproduce pulsos temporizados. El creador del WAV debe asegurar un bucle limpio.',
 '켜면 벽 효과음을 계속 반복합니다. 끄면 정해진 간격으로 재생합니다. WAV 제작자가 반복 경계를 매끄럽게 만들어야 합니다.',
 '開啟時連續循環牆壁音效。關閉時按間隔播放提示音。WAV製作者需確保循環接點平順。',
 '开启时连续循环墙壁音效。关闭时按间隔播放提示音。WAV制作者需确保循环接点平顺。',
 'Вкл. непрерывно зацикливает звук стены. Выкл. воспроизводит сигналы с интервалом. Автор WAV должен обеспечить плавный стык цикла.',
 'Ativado repete o som de parede continuamente. Desativado toca sinais espaçados. O autor do WAV deve garantir uma emenda suave.',
 'เมื่อเปิดจะวนเสียงผนังต่อเนื่อง เมื่อปิดจะเล่นเป็นช่วง ผู้ทำไฟล์ WAV ต้องทำรอยต่อการวนให้เรียบ'),
'Custom repeat interval help':(
 'Custom sounds only: 200 to 5000 milliseconds in steps of 100. Confirm restores original timing. Wall pings then use one second.',
 'カスタム音のみ。200～5000ミリ秒、100ミリ秒刻み。決定で元の間隔に戻ります。壁の単発音は1秒間隔です。',
 'Sons personnalisés : 200 à 5000 millisecondes, par pas de 100. Confirmer rétablit le rythme initial. Murs : une seconde.',
 'Solo suoni personalizzati: da 200 a 5000 millisecondi, passi di 100. Conferma ripristina i tempi originali. Muri: un secondo.',
 'Nur eigene Klänge: 200 bis 5000 Millisekunden in 100er-Schritten. Bestätigen stellt den ursprünglichen Abstand her. Wandsignale dann jede Sekunde.',
 'Solo personalizados: 200 a 5000 milisegundos, pasos de 100. Confirmar restaura el ritmo original. Paredes: un segundo.',
 '사용자 지정 효과음만 적용: 200~5000밀리초, 100씩 조절합니다. 확인하면 기존 간격으로 복원합니다. 벽 알림은 1초입니다.',
 '僅限自訂音效：200到5000毫秒，每次100毫秒。確認可還原原有間隔，此時牆壁提示為1秒。',
 '仅限自定义音效：200到5000毫秒，每次100毫秒。确认可恢复原有间隔，此时墙壁提示为1秒。',
 'Только свои звуки: от 200 до 5000 мс, шаг 100. Подтверждение возвращает исходный интервал. Сигналы стены тогда раз в секунду.',
 'Só personalizados: 200 a 5000 milissegundos, passos de 100. Confirmar restaura o ritmo original. Paredes: um segundo.',
 'เฉพาะเสียงที่กำหนดเอง: 200 ถึง 5000 มิลลิวินาที เพิ่มลดครั้งละ 100 ยืนยันเพื่อคืนจังหวะเดิม เสียงผนังจะใช้หนึ่งวินาที'),
}
def main():
 original=P.read_text(encoding='utf-8');catalog=json.loads(original)
 rows={key:dict(zip(LOCALES,values))for key,values in ROWS.items()}
 assert all(len(values)==12 for values in ROWS.values())
 additions={key:values for key,values in rows.items()if key not in catalog}
 if '--check'in sys.argv:assert not additions,tuple(additions)
 elif additions:
  P.write_text(original[:original.rfind('}')].rstrip()+',\n'+json.dumps(additions,ensure_ascii=False,indent=2)[2:-2]+'\n}\n',encoding='utf-8');catalog.update(additions)
 for key,values in rows.items():
  assert catalog[key]==values,key
  assert all(set(re.findall(r'\{\d+\}',s))==set(re.findall(r'\{\d+\}',key))for s in values.values()),key
 print('Custom audio:',len(rows),'keys x12 languages;',len(additions),'added')
if __name__=='__main__':main()
