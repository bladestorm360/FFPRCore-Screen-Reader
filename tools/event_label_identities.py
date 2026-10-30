"""Reviewed physical identity rules; never export rewards or script outcomes.

Sprite IDs are mapobject master IDs, NOT Tiled editor GIDs. Actual textures were
inspected in the installed asset bundles on October 22; evidence is recorded in
docs/re/OBJECT_IDENTITIES.md. Asset 0 supplies no sprite identity.
"""
import re
import json
import hashlib
from pathlib import Path

# Factual object locators and mod-owned classifications only. Full scripts and
# reconstructed map/sprite sheets remain outside every Git tree.
_review_path = Path(__file__).parent / 'event_identity_reviews.json'
REVIEWS = {(r['game'],r['submap'],r['group'],r['id']):r
           for r in json.loads(_review_path.read_text(encoding='utf-8'))} if _review_path.exists() else {}
WARP_REVIEWS = {(r['game'],r['submap'],r['group'],r['id']):r for r in
               json.loads((Path(__file__).parent/'event_warp_reviews.json').read_text(encoding='utf-8'))}
WORLD_TRAVEL_REVIEWS = {(r['game'],r['submap'],r['raw'],r['script_id']):r for r in
                       json.loads((Path(__file__).parent/'world_transition_reviews.json').read_text(encoding='utf-8'))}


WORLD_EVENT_REVIEWS=json.loads((Path(__file__).parent/'world_event_reviews.json').read_text(encoding='utf-8'))
_functional_scripts={}

def reviewed_event_script(n,obj,script_name,script):
    """Resolve only a pinned review when its script lives in a shared asset.

    Unreviewed inference keeps its existing source rules. Missing/shared script
    ownership cannot silently turn a reviewed transfer back into a scene.
    """
    row=next((r for r in WORLD_EVENT_REVIEWS if (r['game'],r['submap'],r['group'],r['id'])==(n,obj['submap'],obj['group'],obj['id'])
              and r['raw']==obj['name'] and r.get('object_type',2)==obj['type']),None)
    if not row:return script
    if n not in _functional_scripts:
        path=Path(__file__).resolve().parents[2]/'tools/mapdump'/f'ff{n}_scripts.jsonl'
        _functional_scripts[n]=[json.loads(line) for line in path.open(encoding='utf-8-sig')]
    sources=_functional_scripts[n]
    if row.get('source_variants'):
        actual={hashlib.sha256(json.dumps(s.get('mnemonics'),sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
                for s in sources if s['name']==script_name}
        assert actual==set(row['source_variants']),'Reviewed global script variants changed'
    if not script:
        candidates=[s for s in sources if s['name']==script_name and
            hashlib.sha256(json.dumps(s,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()==row['script_sha256']]
        assert candidates,'Reviewed shared script missing or changed'
        script=candidates[0]
    if row.get('action_id',2)!=0:
        # Native GetScriptData tries the loaded map and resident script bank.
        # A same-named body in another map is not executable source evidence.
        # Current reviewed roots are all exact local or Resident bindings.
        assert script and script['submap']in(obj['submap'],'Resident'),'Reviewed source is outside the native local/resident bank'
    for child in row.get('continuations',[]):
        assert any(s['submap']==child['submap'] and s['name']==child['script'] and
            hashlib.sha256(json.dumps(s.get('mnemonics'),sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()==child['body_sha256']
            for s in sources),'Reviewed travel continuation changed'
    return script


def reviewed_world_event_identity(n, obj, script_name, script):
    # Exact occurrences and whole-script hashes: reviewed entrances, barriers
    # and bookkeeping keep functional identities without revealing story scenes.
    row=next((r for r in WORLD_EVENT_REVIEWS if (r['game'],r['submap'],r['group'],r['id'])==(n,obj['submap'],obj['group'],obj['id'])
              and r['raw']==obj['name'] and r.get('object_type',2)==obj['type']),None)
    if not row:return None
    p=obj.get('props') or {}
    assert obj['type']==row.get('object_type',2) and p.get('action_id')==row.get('action_id',2) and row['raw']==obj['name']
    assert row['entity_id']==p.get('entity_id') and row['script_id']==p.get('script_id') and row['script_name']==script_name
    digest=hashlib.sha256(json.dumps(script,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()
    assert digest==row['script_sha256'],'Reviewed world event changed; inspect before classifying'
    return row['label'],row['reason'],None


def reviewed_warp_identity(n, obj, script_name, script):
    """Reviewed travel pads, never all scripts which happen to change maps.

    Pandaemonium's generic numbered scripts only play the warp animation/sound,
    fade, optionally maintain the teleport cache and change maps. Their native
    identifiers often omit 'warp'. Bind the complete inspected script and exact
    occurrence so narrative scenes, traps and shared numbered names stay separate.
    Only facts/hashes are kept here; script bodies remain outside Git.
    """
    row = WARP_REVIEWS.get((n,obj['submap'],obj['group'],obj['id']))
    if not row:
        return None
    p = obj.get('props') or {}
    assert obj['type'] == 2 and p.get('action_id') == 2, 'Reviewed warp activation changed'
    assert row['raw'] == obj['name'] and row['entity_id'] == p.get('entity_id')
    assert row['script_id'] == p.get('script_id') and row['script_name'] == script_name
    digest = hashlib.sha256(json.dumps(script,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode('utf-8')).hexdigest()
    assert row['script_sha256'] == digest, 'Reviewed warp script changed; inspect before relabelling'
    return 'Warp Tile', 'reviewed warp animation and direct map travel', None
_subjects = {}
_chest_copies={(r['game'],r['submap'],r['layer'],*r['evidence']['tile']) for r in REVIEWS.values() if r['label']=='Treasure Chest'}
for _r in REVIEWS.values():
    # Named/described repeat instances may share a reviewed physical subject.
    # Empty containers cannot: one empty-message script may serve pots, barrels
    # and chests. Those remain exact occurrences until their own art is checked.
    if _r['label'] in {'Pot','Barrel','Treasure Chest'} and not (
        _r['game']==6 and _r['label']=='Treasure Chest' and _r['raw'] in
        {'カラッポ！','カラッポ！_開封済み','カラッポ！_開封後'}): continue
    _key=(_r['game'],_r['submap'],_r['raw'],_r['script_id'],_r['message_key'],_r['asset_id'])
    _subjects.setdefault(_key,set()).add(_r['label'])

def reviewed_identity(n, obj):
    row=REVIEWS.get((n,obj['submap'],obj['group'],obj['id']))
    p=obj.get('props',{})
    if not row:
        tile_y=int((obj['y']-obj['h'])//16) if obj.get('gid') else obj['ty']
        if n==6 and obj['name'].startswith('カラッポ！_開封') and (n,obj['submap'],obj['layer'],obj['tx'],tile_y) in _chest_copies:
            return 'Treasure Chest', 'reviewed physical opened-chest counterpart', None
        labels=_subjects.get((n,obj['submap'],obj['name'],p.get('script_id',0),p.get('message_key',''),p.get('asset_id',0)),set())
        return (next(iter(labels)), 'reviewed physical interaction subject copies', None) if len(labels)==1 else None
    assert row['raw']==obj['name'] and all(row[k]==p.get(k, '' if k=='message_key' else -1 if k=='entity_id' else 0)
        for k in ('entity_id','script_id','message_key','asset_id')), 'Reviewed identity source changed'
    return row['label'], 'reviewed physical map/script occurrence', row['asset_id'] or None

SPRITES = {
    1: {103:'Submarine',106:'Orb',107:'Orb',109:'Orb',111:'Orb',113:'Orb',
        117:'Crystal',119:'Rock',128:'Crystal',138:'Warp Tile',144:'Tablet'},
    2: {69:'Orb',72:'Iron gate',157:'Orb',173:'Ship',175:'Ladder'},
    3: {124:'Crystal',126:'Orb',127:'Horn',130:'Lute',131:'Fang',132:'Staff',133:'Sword',
        134:'Sword',135:'Chakram',136:'Sword',138:'Crystal',217:'Grass',218:'Rock'},
    4: {137:'Crystal',140:'Crystal',141:'Crystal',142:'Sword',143:'Sword',
        144:'Spear',145:'Guillotine',310:'Airship',346:'Airship'},
    5: {95:'Crystal',101:'Rock',105:'Book',106:'Book',107:'Map',108:'Letter',117:'Tablet'},
    6: {94:'Letter',96:'Crystal',103:'Statue',106:'Machine',151:'Raft',157:'Weight',265:'Diary'},
}

# These identify the physical subject of the interaction, not objects mentioned
# in a future event. Qualified developer names may describe an animation/action;
# only the noun is spoken. An unnamed script that opens a door is insufficient.
RULES = (
    ('Iron gate',r'^(?:上がる|開く|開かない)?鉄格子'),
    ('Door',r'^(?:(?:カギ|鍵)のかか(?:った|っている)扉|(?:カギ|鍵)がかか|鍵が閉まっている|扉に鍵|扉コリジョン)'),
    ('Piano',r'^(?:ピアノ(?:演奏|を弾く)|.*のピアノを弾く)'),
    ('Bed',r'^(?:回復ベッド|ベッド(?:で|を|[（(]|$)|.*のベッドで寝る|布団で休む)'),
    ('Bookshelf',r'^(?:.*の)?本棚(?:を調べる|が動く|$)'),
    ('Book',r'^(?:図書館の本を読む|本(?:を読|[（(]|$)|開いた本|閉じた本|開いている本|アルテマの本)'),
    ('Letter',r'^(?:.*の)?(?:手紙|置手紙|便箋)(?:を読|$)'),
    ('Diary',r'^(?:.*の)?日記(?:を読む|$)'),
    ('Lever',r'^(?:レバー(?:操作|切り替え|上げ下げ|を押|$)|飛行モードにするレバー)'),
    ('Switch',r'^(?:壁スイッチ|隠しスイッチ|壁のドクロ(?:の)?スイッチ|スイッチ(?:を押|押す|踏む|$))'),
    ('Statue',r'^(?:女神像|三闘神の像|石像[（(])'),
    ('Fountain',r'^(?:回復の泉|生き返りの泉|命の泉|泉を調べる)'),
    ('Pot',r'^回復の(?:壺|ツボ)'),
    ('Bucket',r'^バケツ(?:に触|$)'),
    ('Tablet',r'^石板(?:[（(]|$)'),
    ('Crystal',r'^(?:(?:水|土|火|風|闇)の)?クリスタル(?:[（(]|のかけら|の欠片|かけら|青|$)'),
    ('Window',r'^展望窓を覗く'),
    ('Orb',r'^(?:黒色|土色|赤色|水色)の玉|^(?:精神|知性|素早さ|力)が上がるオーブ'),
    ('Guillotine',r'^ギロチン$'),
    ('Candle',r'^ロウソク(?:[①-⑳\d]|$)'),
    ('Hole',r'^小さな穴$'),
    ('Chain',r'^チェーン(?:で|のぼ|降り|昇る)'),
    ('Vine',r'^ツタ(?:を上る|のぼり)'),
    ('Rock',r'^(?:岩|石)を飛(?:んでわたる|ぶ)'),
    ('Ladder',r'^梯子(?:昇降|を下りて)'),
    ('Gravestone',r'^(?:[①-⑳\d]*お墓|墓の説明|墓標|石碑を調べる)'),
    ('Campfire',r'^焚火'),
    ('Fireplace',r'^暖炉$'),
    ('Stove',r'^ストーブ'),
    ('Kettle',r'^やかん$'),
    ('Painting',r'^.+の絵$'),
    ('Toilet',r'^調べるとトイレの水'),
    ('Moving platform',r'^(?:通路が動く|移動床(?:に|で))'),
    ('Elevator',r'^エレベーター'),
    ('Escalator',r'^エスカレーター'),
    ('Crane',r'^クレーンで降りる'),
    ('Stairs',r'^階段を(?:上る|降りる)'),
    ('Rock',r'^(?:通路をふさぐ岩|岩が道をふさいでいる|石の上を飛ぶ)'),
    ('Hidden passage',r'^隠し通路(?:入口|$)'),
    ('Helm',r'^(?:インビンシブルを操作する|魔導船発進|飛空艇を操縦|飛空艇を発進するか選択)'),
    ('Flame',r'^(?:太陽の炎トリガー用|炎スクリプト|炎のアニメ)'),
    ('Well',r'^アガルトの村の井戸で'),
    ('Mirror',r'^鏡の警告'),
    ('Weapon rack',r'^スミスの作品$'),
    ('Egg',r'^飛竜の卵$'),
    ('Newspaper',r'^新聞$'),
    ('Poster',r'^ポスター$'),
    ('Treasure Chest',r'^(?:空の宝箱|宝箱[１２３４５６1-6]|移動宝箱|光る宝箱スイッチ)$'),
)

def physical_identity(n, obj):
    if obj['type'] not in (5,10): return None
    asset = obj.get('props',{}).get('asset_id',0)
    label = SPRITES[n].get(asset)
    if label: return label, 'reviewed mapobject sprite', asset
    raw = obj.get('name','').strip()
    raw = re.sub(r'^(?:sc_|ev_)[^:：]*[:：]', '', raw)
    raw = re.sub(r'^(?:sc|ev|se)_[a-z]_\d+(?:_\d+)*\s*', '', raw)
    for label, pattern in RULES:
        if re.search(pattern,raw):return label, 'reviewed physical interaction subject', None
    return None

def script_identity(n, obj, script_name, messages):
    """Only inspected physical subjects; no reward or future action is a name."""
    p=obj.get('props') or {}
    travel=WORLD_TRAVEL_REVIEWS.get((n,obj['submap'],obj.get('name',''),p.get('script_id')))
    if travel:
        # A repeated world border is real conditional map travel, not animation
        # scenery. Identity is family-specific; other scenes keep their labels.
        assert obj['type']==2 and p.get('action_id')==2 and obj['group']==travel['group']
        assert script_name==travel['script_name']
        return 'World map transition','reviewed airship world-border transition family',None
    if obj['type'] not in (5,10): return None
    # The reviewed Phoenix Tower interaction copies operate the same physical
    # wall panels on its other floors. Do not speak their future battle/stair
    # outcomes as though those were the visible object's name.
    if n==5 and obj['submap'].startswith('Map_30541_') and obj['name'].startswith('階段が出現'):
        return 'Wall', 'reviewed physical Phoenix Tower wall-panel copies', None
    if n==3 and (obj['name'].startswith('デブチョコボ') or obj['name']=='ギサールのやさいを使う') and script_name:
        return 'Chocobo clearing', 'reviewed physical Chocobo summon clearing', None
    if script_name.startswith('sc_sty_'):
        return 'Bed', 'reviewed physical rest interaction', None
    # Native text markup is formatting, not part of the object's noun. Matching
    # stays anchored to concrete physical inspection text, never an arbitrary
    # noun later in a conversation or an obtained-item notification.
    patterns = (
        ('Gravestone',r"^(?:It's a gravestone\.|Here lies Link\.)"),
        ('Campfire',r"^It's a campfire\."),
        ('Door',r'^(?:This door has been secured|The door is (?:locked|sealed)|The door is rusted shut)'),
        ('Iron gate',r'^An iron gate blocks the doorway'),
        ('Rock',r'^(?:A (?:large stone|boulder) blocks|Josef turns a small rock jutting out of the wall)'),
        ('Engine',r"^You see the Dreadnought's engine\."),
        ('Flame',r'^(?:The Sunfire blazes brightly|A wall of fire is obstructing the path|The flames lick and flare)'),
        ('Piano',r'^Play the piano\?'),
        ('Statue',r"^It's a statue of the Nepto Dragon"),
        ('Bookshelf',r'^[A-Z]-[A-Z] Shelf$'),
        ('Painting',r'^(?:A picture of an old urn|A treasure chest on canvas|A beautiful still life of flowers|It appears to be a lamp|A very old painting|A suit of armor, it would seem|A picture of a lovely lady)'),
    )
    found=set()
    for message in messages:
        text=re.sub(r'<[^>]*>','',message).strip()
        for label,pattern in patterns:
            if re.match(pattern,text):found.add(label)
    if len(found)==1:return found.pop(), 'reviewed physical interaction message', None
    return None
