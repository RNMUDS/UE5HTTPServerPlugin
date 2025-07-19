# UE5HTTPServer Plugin

Unreal Engine 5用のHTTPサーバープラグインです。REST APIを通じてUE5のアクターやシーンをリアルタイムで制御できます。

## 機能

- REST APIでアクターの作成・編集・削除
- シーン情報の取得
- アクターの位置・回転・スケール・色の変更
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
| GET | `/actors/{name}` | 特定のアクター情報を取得 |
| PUT | `/actors/{name}` | アクターを更新 |
| DELETE | `/actors/{name}` | アクターを削除 |
| PUT | `/actors/{name}/location` | 位置のみ更新 |
| PUT | `/actors/{name}/rotation` | 回転のみ更新 |
| PUT | `/actors/{name}/scale` | スケールのみ更新 |
| PUT | `/actors/{name}/color` | 色のみ更新 |

## MCP (Model Context Protocol) 対応

このプラグインはMCPをサポートしており、AIアシスタントツールから直接UE5を制御できます。

## ライセンス

MIT License

## 貢献

Issue報告やプルリクエストを歓迎します！

## 作者

Created with [Claude Code](https://claude.ai/code)