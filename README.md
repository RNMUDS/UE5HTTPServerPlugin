# UE5HTTPServer Plugin

Unreal Engine 5用のHTTPサーバープラグインです。REST APIを通じてUE5のアクターやシーンをリアルタイムで制御できます。

## 機能

- REST APIでアクターの作成・編集・削除
- アクターの位置・回転・スケール・色の変更
- MCP (Model Context Protocol) 対応

## UE5プラグインの作成

1. UE5で新規プロジェクトを作成
  - Engineのバージョン5.5で起動
  - ゲーム　→　ブランク → C++ → スタートコンテンツのチェック外す
  - プロジェクト名を「UE5MCPProject」に設定し、作成
2. このプラグインをプロジェクトの `Plugins` フォルダーにコピー
3. UE5で上記で作成したプロジェクトを開く
4. UE5のプロジェクトをいったん閉じる
5. ターミナルで上記で作成したプロジェクトを開く
  - 例：　~/Documents/Unreal Projects/UE5MCPProject
6. 下のコマンドを実行し、ビルド
```
"/Users/Shared/Epic Games/UE_5.5/Engine/Build/BatchFiles/Mac/Build.sh" \
  UE5MCPProjectEditor \
  Mac \
  Development \
  -project="$(pwd)/UE5MCPProject.uproject" \
  -waitmutex
```
下のように表示されたら成功
```
Deploying UE5MCPProjectEditor Mac Development...
Deploying now!
Total execution time: 5.52 seconds
```

## 使い方（ターミナルからUE5にオブジェクト生成）

### 1. サーバーの起動

作成のUE5のプロジェクト「UE5MCPProject」のエディタを起動すると、自動的にHTTPサーバがポート8080で起動します。

### 2. ヘルスチェック

ターミナルで以下のコマンドを実行し、動作確認
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
## MCPサーバの作成（Claude Desktopから自然言語でUE5にオブジェクト生成）
たとえばUE5MCPProjectの中にMCP_serverディレクトリを作成し、その中で作業
1. 仮想環境構築
```bash
python -m venv .venv
source .venv/bin/activate
pip install mcp httpx
```
2. このディレクトリにue5_server.pyを配置
3. Claude Desktop設定を編集
```bash
nano ~/Library/Application\ Support/Claude/claude_desktop_config.json
```
以下のように追記（pythonとue5_mcp_server.pyのパスを適宜変更）
```
{
  "mcpServers": {
    "ue5-control": {
      "command": "/Users/rn/.pyenv/shims/python",
      "args": ["/Users/rn/Documents/Unreal Projects/UE5MCPProject/MCP_server/ue5_mcp_server.py"],
      "env": {
        "UE5_SERVER_URL": "http://localhost:8080"
      }
    }
  }
}
```
4. Claude Desktopを再起動、エラーなく起動できることを確認