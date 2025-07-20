# UE5HTTPServer Plugin

Unreal Engine 5用のHTTPサーバープラグインです。REST APIを通じてUE5のアクターやシーンをリアルタイムで制御できます。

## 機能

- REST APIでアクターの作成・編集・削除
- バッチ作成機能（複数アクターの一括作成）
- グリッド作成機能（規則的な配置でアクターを生成）
- 全アクター削除機能
- シーン情報の取得
- アクターの位置・回転・スケール・色の変更
- 個別アクターの詳細なサイズ指定（dimensions/scale）
- ライトの制御（色・強度・減衰半径）
- MCP (Model Context Protocol) 対応

## インストール

1. このプラグインをプロジェクトの `Plugins` フォルダーにコピー
2. UE5でプロジェクトを開く
3. プラグインが自動的に有効化されます

## 使い方

### 1. サーバーの起動

PIE (Play In Editor) モードでゲームを実行すると、HTTPサーバーが自動的にポート8080で起動します。

### 2. ヘルスチェック

```bash
curl http://localhost:8080/health
```

### 3. アクターの作成

```bash
# 赤いキューブを作成
curl -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cube",
    "name": "RedCube",
    "location": {"x": -200, "y": 0, "z": 100},
    "color": {"r": 1.0, "g": 0.1, "b": 0.1}
  }'

# 青い球体を作成
curl -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Sphere",
    "name": "BlueSphere",
    "location": {"x": 0, "y": 0, "z": 100},
    "color": {"r": 0.1, "g": 0.1, "b": 1.0},
    "scale": {"uniform": 1.5}
  }'
```

### 4. アクターの更新

```bash
# 位置を変更
curl -X PUT http://localhost:8080/actors/BlueSphere/location \
  -H "Content-Type: application/json" \
  -d '{"location": {"x": 0, "y": 200, "z": 100}}'

# 回転を変更
curl -X PUT http://localhost:8080/actors/RedCube/rotation \
  -H "Content-Type: application/json" \
  -d '{"rotation": {"pitch": 0, "yaw": 45, "roll": 0}}'
```

### 5. シーン情報の取得

```bash
curl http://localhost:8080/scene
```

### 6. アクターの削除

```bash
curl -X DELETE http://localhost:8080/actors/RedCube
```

## クイックテストスクリプト

以下のスクリプトで基本的な動作確認ができます：

```bash
#!/bin/bash

# UE5 HTTPサーバー クイックテスト
# このスクリプトで基本的な動作確認ができます

echo "🚀 UE5 HTTPサーバー クイックテスト開始"
echo "================================"

# 1. ヘルスチェック
echo -e "\n✅ ヘルスチェック..."
if curl -s http://localhost:8080/health | grep -q "ok"; then
    echo "サーバーは正常に動作しています"
else
    echo "❌ サーバーに接続できません。UE5でPIEモードが起動していることを確認してください。"
    exit 1
fi

# 2. カラフルなデモシーン作成
echo -e "\n🎨 カラフルなデモシーンを作成中..."

# 床を作成
echo "  - グレーの床を作成"
curl -s -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Plane",
    "name": "Floor",
    "location": {"x": 0, "y": 0, "z": 0},
    "color": {"r": 0.3, "g": 0.3, "b": 0.3},
    "scale": {"uniform": 20.0}
  }' > /dev/null

# 赤いキューブ
echo "  - 赤いキューブを作成"
curl -s -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cube",
    "name": "RedCube",
    "location": {"x": -200, "y": 0, "z": 100},
    "color": {"r": 1.0, "g": 0.1, "b": 0.1}
  }' > /dev/null

# 青い球体
echo "  - 青い球体を作成"
curl -s -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Sphere",
    "name": "BlueSphere",
    "location": {"x": 0, "y": 0, "z": 100},
    "color": {"r": 0.1, "g": 0.1, "b": 1.0},
    "scale": {"uniform": 1.5}
  }' > /dev/null

# 緑のシリンダー
echo "  - 緑のシリンダーを作成"
curl -s -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cylinder",
    "name": "GreenCylinder",
    "location": {"x": 200, "y": 0, "z": 100},
    "color": {"r": 0.1, "g": 1.0, "b": 0.1},
    "scale": {"x": 1.0, "y": 1.0, "z": 2.0}
  }' > /dev/null

# オレンジのライト
echo "  - オレンジ色のライトを作成"
curl -s -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Light",
    "name": "OrangeLight",
    "location": {"x": 0, "y": 0, "z": 400},
    "color": {"r": 1.0, "g": 0.6, "b": 0.2},
    "intensity": 5000,
    "attenuationRadius": 1000
  }' > /dev/null

# 3. アニメーション風の動き
echo -e "\n🎬 アニメーションデモ..."
echo "  - RedCubeを回転"
for angle in 0 45 90 135 180; do
    curl -s -X PUT http://localhost:8080/actors/RedCube/rotation \
      -H "Content-Type: application/json" \
      -d "{\"rotation\": {\"pitch\": 0, \"yaw\": $angle, \"roll\": 0}}" > /dev/null
    sleep 0.2
done

echo "  - BlueSphereを移動"
for y in 0 100 200 100 0; do
    curl -s -X PUT http://localhost:8080/actors/BlueSphere/location \
      -H "Content-Type: application/json" \
      -d "{\"location\": {\"x\": 0, \"y\": $y, \"z\": 100}}" > /dev/null
    sleep 0.2
done

# 4. 現在のシーン情報を表示
echo -e "\n📊 現在のシーン情報:"
curl -s http://localhost:8080/scene | python3 -c "
import json, sys
data = json.load(sys.stdin)
print(f\"  アクター数: {data['actorCount']}\")
for actor in data['actors'][:5]:  # 最初の5個だけ表示
    print(f\"  - {actor['name']} ({actor['class']})\")
if data['actorCount'] > 5:
    print(f\"  ... 他 {data['actorCount'] - 5} 個のアクター\")
"

echo -e "\n✨ テスト完了！"
echo "================================"
echo "💡 ヒント:"
echo "  - UE5のビューポートでカメラを動かして作成されたオブジェクトを確認してください"
echo "  - 個別のcurlコマンドは ue5-curl-simple.md を参照してください"
echo "  - クリーンアップするには各アクターをDELETEメソッドで削除してください"
```

## バッチ作成機能の使用例

### 個別アクターの詳細なサイズ指定

#### dimensionsを使った物理的なサイズ指定
```bash
# 幅200、奥行き100、高さ50のキューブを作成
curl -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cube",
    "name": "CustomSizeCube",
    "location": {"x": 0, "y": 0, "z": 100},
    "dimensions": {"width": 200, "depth": 100, "height": 50}
  }'

# 直径300の球体を作成
curl -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Sphere",
    "name": "LargeSphere",
    "location": {"x": 300, "y": 0, "z": 100},
    "dimensions": {"width": 300, "depth": 300, "height": 300}
  }'
```

#### scaleを使った倍率指定
```bash
# 2倍の大きさのキューブを作成
curl -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cube",
    "name": "DoubleSizeCube",
    "location": {"x": 0, "y": 0, "z": 100},
    "scale": {"uniform": 2}
  }'

# X方向に3倍、Y方向に2倍に伸ばしたシリンダーを作成
curl -X POST http://localhost:8080/actors \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cylinder",
    "name": "StretchedCylinder",
    "location": {"x": -300, "y": 0, "z": 100},
    "scale": {"x": 3, "y": 2, "z": 1}
  }'
```

### バッチ作成

#### 複数のアクターを一度に作成
```bash
curl -X POST http://localhost:8080/actors/batch \
  -H "Content-Type: application/json" \
  -d '{
    "actors": [
      {
        "type": "Cube",
        "name": "RedCube",
        "location": {"x": 0, "y": 0, "z": 100},
        "color": {"r": 1, "g": 0, "b": 0},
        "dimensions": {"width": 200, "depth": 200, "height": 200}
      },
      {
        "type": "Sphere",
        "name": "BlueSphere",
        "location": {"x": 300, "y": 0, "z": 100},
        "color": {"r": 0, "g": 0, "b": 1},
        "dimensions": {"width": 150, "depth": 150, "height": 150}
      },
      {
        "type": "Cylinder",
        "name": "GreenCylinder",
        "location": {"x": -300, "y": 0, "z": 100},
        "color": {"r": 0, "g": 1, "b": 0},
        "dimensions": {"width": 100, "depth": 100, "height": 300}
      }
    ]
  }'
```

### グリッド作成

#### 基本的なグリッド
```bash
# 5x5のキューブグリッドを作成
curl -X POST http://localhost:8080/actors/grid \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cube",
    "rows": 5,
    "columns": 5,
    "spacing": 150,
    "startLocation": {"x": 0, "y": 0, "z": 100}
  }'

# 10x10の球体グリッドを作成（全て赤色）
curl -X POST http://localhost:8080/actors/grid \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Sphere",
    "rows": 10,
    "columns": 10,
    "spacing": 100,
    "startLocation": {"x": 0, "y": 0, "z": 100},
    "color": {"r": 1, "g": 0, "b": 0}
  }'
```

#### 詳細なグリッド
```bash
# 8x8のキューブグリッドを詳細設定で作成
curl -X POST http://localhost:8080/actors/grid \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Cube",
    "rows": 8,
    "columns": 8,
    "spacing": 200,
    "startLocation": {"x": -800, "y": -800, "z": 50},
    "color": {"r": 0, "g": 0.2, "b": 1},
    "dimensions": {"width": 100, "depth": 100, "height": 150}
  }'
```

### 全アクター削除
```bash
# シーンの全てのアクターを削除
curl -X DELETE http://localhost:8080/actors
```

## 実用的な使用例

### 建物の壁を一度に作成
```bash
curl -X POST http://localhost:8080/actors/batch \
  -H "Content-Type: application/json" \
  -d '{
    "actors": [
      {
        "type": "Cube",
        "name": "FrontWall",
        "location": {"x": 0, "y": -500, "z": 250},
        "dimensions": {"width": 1000, "depth": 20, "height": 500},
        "color": {"r": 0.5, "g": 0.5, "b": 0.5}
      },
      {
        "type": "Cube",
        "name": "BackWall",
        "location": {"x": 0, "y": 500, "z": 250},
        "dimensions": {"width": 1000, "depth": 20, "height": 500},
        "color": {"r": 0.5, "g": 0.5, "b": 0.5}
      },
      {
        "type": "Cube",
        "name": "LeftWall",
        "location": {"x": -500, "y": 0, "z": 250},
        "dimensions": {"width": 20, "depth": 1000, "height": 500},
        "color": {"r": 0.5, "g": 0.5, "b": 0.5}
      },
      {
        "type": "Cube",
        "name": "RightWall",
        "location": {"x": 500, "y": 0, "z": 250},
        "dimensions": {"width": 20, "depth": 1000, "height": 500},
        "color": {"r": 0.5, "g": 0.5, "b": 0.5}
      }
    ]
  }'
```

### ライトグリッド
```bash
# 天井照明として4x4のライトグリッドを作成
curl -X POST http://localhost:8080/actors/grid \
  -H "Content-Type: application/json" \
  -d '{
    "type": "Light",
    "rows": 4,
    "columns": 4,
    "spacing": 400,
    "startLocation": {"x": -600, "y": -600, "z": 500},
    "color": {"r": 1, "g": 0.9, "b": 0.7},
    "intensity": 3000,
    "attenuationRadius": 500
  }'
```

## パフォーマンスの考慮事項

- 一度に作成するアクターは100個程度まで推奨
- 大量のアクター作成時は、PIEモードのパフォーマンスに注意
- グリッド作成は最大20x20程度を推奨

## サポートされているアクタータイプ

- **Cube**: 立方体
- **Sphere**: 球体
- **Cylinder**: シリンダー
- **Plane**: 平面
- **Light**: ポイントライト

## APIエンドポイント一覧

| メソッド | エンドポイント | 説明 |
|---------|---------------|------|
| GET | `/health` | ヘルスチェック |
| GET | `/scene` | シーン情報を取得 |
| POST | `/actors` | 新しいアクターを作成 |
| POST | `/actors/batch` | 複数のアクターを一括作成 |
| POST | `/actors/grid` | グリッド状にアクターを作成 |
| DELETE | `/actors` | 全てのアクターを削除 |
| GET | `/actors/{name}` | 特定のアクター情報を取得 |
| PUT | `/actors/{name}` | アクターを更新 |
| DELETE | `/actors/{name}` | アクターを削除 |
| PUT | `/actors/{name}/location` | 位置のみ更新 |
| PUT | `/actors/{name}/rotation` | 回転のみ更新 |
| PUT | `/actors/{name}/scale` | スケールのみ更新 |
| PUT | `/actors/{name}/color` | 色のみ更新 |

## MCP (Model Context Protocol) 対応

このプラグインはMCPをサポートしており、AIアシスタントツールから直接UE5を制御できます。

### Claude Desktop設定

MCPサーバー（ue5-server.py）をClaude Desktopで使用するための設定方法：

#### 1. Pythonパスの確認

まず、使用するPythonのパスを確認します：
```bash
which python3
# または
which python
```

#### 2. Claude Desktopの設定ファイルを開く

```bash
# macOSの場合
open ~/Library/Application\ Support/Claude/

# 設定ファイルを編集
nano ~/Library/Application\ Support/Claude/claude_desktop_config.json
```

#### 3. 設定ファイルの編集

設定ファイル（`claude_desktop_config.json`）を以下のように編集します：

**新規作成の場合：**
```json
{
  "mcpServers": {
    "ue5-control": {
      "command": "/usr/bin/python3",
      "args": ["/Users/rn/Documents/Unreal Projects/UE5MCPProject/Plugins/ue5-server.py"],
      "env": {
        "UE5_SERVER_URL": "http://localhost:8080"
      }
    }
  }
}
```

**既存の設定がある場合：**
```json
{
  "mcpServers": {
    "existing-server": {
      // 既存の設定...
    },
    "ue5-control": {
      "command": "/usr/bin/python3",
      "args": ["/Users/rn/Documents/Unreal Projects/UE5MCPProject/Plugins/ue5-server.py"],
      "env": {
        "UE5_SERVER_URL": "http://localhost:8080"
      }
    }
  }
}
```

**重要な設定項目：**
- **`command`**: `which python3`で確認したPythonの実行パスを指定
  - 例: `/usr/bin/python3`, `/usr/local/bin/python3`, `/opt/homebrew/bin/python3`など
- **`args`**: ue5-server.pyファイルの完全パスを指定
  - 上記の例では `/Users/rn/Documents/Unreal Projects/UE5MCPProject/Plugins/ue5-server.py`
  - スペースを含むパスの場合もそのまま記述（エスケープ不要）
- **`env`**: 環境変数の設定
  - `UE5_SERVER_URL`: UE5 HTTPサーバーのURL（デフォルト: `http://localhost:8080`）

**よくある設定ミス：**
- ❌ パスにエスケープ文字を使用: `"\/Users\/rn\/..."`
- ❌ 相対パスを使用: `"./ue5-server.py"`
- ❌ カンマの付け忘れ（複数のMCPサーバーがある場合）
- ✅ 正しい例: `"/Users/rn/Documents/Unreal Projects/UE5MCPProject/Plugins/ue5-server.py"`

#### 4. 設定の確認

1. **Claude Desktopを完全に終了して再起動**
   - メニューバーのClaudeアイコンから「Quit Claude」を選択
   - 再度Claude Desktopを起動

2. **MCPサーバーの認識確認**
   - Claude Desktopの設定画面でMCPサーバーが表示されているか確認
   - エラーが表示されている場合は、パスやPythonの設定を再確認

3. **動作テスト**
   - UE5でPIEモードを起動（HTTPサーバーが自動的に起動）
   - Claude Desktopで以下のようなコマンドを試す：
     - 「UE5でキューブを作成して」
     - 「赤い球体を位置(100, 0, 100)に作成」
     - 「シーンの情報を教えて」

#### トラブルシューティング

**MCPサーバーが認識されない場合：**
1. 設定ファイルのJSON形式が正しいか確認（JSONバリデータを使用）
2. Pythonパスが正しいか再確認: `which python3`
3. ue5-server.pyファイルが指定した場所に存在するか確認
4. Claude Desktopのログを確認

**「コマンドが見つからない」エラーの場合：**
```bash
# Pythonがインストールされているか確認
python3 --version

# 必要に応じてPythonをインストール
# macOS (Homebrew使用)
brew install python3
```

**設定ファイルの場所が分からない場合：**
```bash
# macOS
ls ~/Library/Application\ Support/Claude/

# ファイルが存在しない場合は新規作成
touch ~/Library/Application\ Support/Claude/claude_desktop_config.json
```

## ライセンス

MIT License

## 貢献

Issue報告やプルリクエストを歓迎します！

## 作者

Created with [Claude Code](https://claude.ai/code)