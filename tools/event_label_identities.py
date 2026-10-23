"""Reviewed physical identity rules; never export rewards or script outcomes.

Sprite IDs are mapobject master IDs, NOT Tiled editor GIDs. Actual textures were
inspected in the installed asset bundles on October 22; evidence is recorded in
docs/re/OBJECT_IDENTITIES.md. Asset 0 supplies no sprite identity.
"""
import re
import json
from pathlib import Path

# Factual object locators and mod-owned classifications only. Full scripts and
# reconstructed map/sprite sheets remain outside every Git tree.
_review_path = Path(__file__).parent / 'event_identity_reviews.json'
REVIEWS = {(r['game'],r['submap'],r['group'],r['id']):r
           for r in json.loads(_review_path.read_text(encoding='utf-8'))} if _review_path.exists() else {}
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
