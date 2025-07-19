# UE5 MCP Project with HTTP Server

Unreal Engine 5プロジェクト with HTTPサーバープラグイン。外部アプリケーションからHTTP APIを使ってUE5のシーンを操作できます。

## 機能

- HTTP APIによるアクターの作成・削除・更新
- リアルタイムなシーン情報の取得
- プリミティブ形状（Cube, Sphere, Cylinder, Plane）の生成
- ライトの制御
- アクターの位置・回転・スケール・色の変更

## セットアップ

1. Unreal Engine 5.4以降をインストール
2. このプロジェクトをクローン
   ```bash
   git clone https://github.com/RNMUDS/UE5MCPProject.git
   ```
3. `.uproject`ファイルをダブルクリックして開く
4. PIE（Play In Editor）モードで実行

## クイックテスト

プロジェクトに含まれているテストスクリプトを実行：

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

## API エンドポイント

### ヘルスチェック
```bash
GET http://localhost:8080/health
```

### アクター作成
```bash
POST http://localhost:8080/actors
Content-Type: application/json

{
  "type": "Cube",
  "name": "MyCube",
  "location": {"x": 0, "y": 0, "z": 100},
  "color": {"r": 1.0, "g": 0.0, "b": 0.0}
}
```

### アクター一覧取得
```bash
GET http://localhost:8080/actors
```

### アクター情報取得
```bash
GET http://localhost:8080/actors/{actorName}
```

### アクター削除
```bash
DELETE http://localhost:8080/actors/{actorName}
```

### 位置更新
```bash
PUT http://localhost:8080/actors/{actorName}/location
Content-Type: application/json

{
  "location": {"x": 100, "y": 0, "z": 100}
}
```

### 回転更新
```bash
PUT http://localhost:8080/actors/{actorName}/rotation
Content-Type: application/json

{
  "rotation": {"pitch": 0, "yaw": 45, "roll": 0}
}
```

### スケール更新
```bash
PUT http://localhost:8080/actors/{actorName}/scale
Content-Type: application/json

{
  "scale": {"x": 2.0, "y": 2.0, "z": 2.0}
}
```

### シーン情報取得
```bash
GET http://localhost:8080/scene
```

## サポートされているアクタータイプ

- **Cube**: 立方体
- **Sphere**: 球体
- **Cylinder**: シリンダー
- **Plane**: 平面
- **Light**: ポイントライト

## 必要環境

- Unreal Engine 5.4以降
- Windows 10/11 または macOS
- Visual Studio 2022（Windows）または Xcode（macOS）

## ライセンス

このプロジェクトはMITライセンスで公開されています。

## 貢献

プルリクエストは歓迎します！バグ報告や機能リクエストはIssuesへお願いします。

## 注意事項

- 大きなアセット（69MB）が含まれているため、将来的にGit LFSの使用を検討してください
- HTTPサーバーはPIEモード実行中のみ有効です
- デフォルトポートは8080です