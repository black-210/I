#!/usr/bin/env bash
set -e

echo "==> Building Language I VS Code VSIX Extension Package..."

npx esbuild vscode-extension/src/extension.ts \
  --bundle \
  --outfile=vscode-extension/dist/extension.js \
  --external:vscode \
  --format=cjs \
  --platform=node

rm -rf /tmp/vsix_pack
mkdir -p /tmp/vsix_pack/extension

cp "vscode-extension/[Content_Types].xml" /tmp/vsix_pack/
cp vscode-extension/extension.vsixmanifest /tmp/vsix_pack/
cp vscode-extension/package.json /tmp/vsix_pack/extension/
cp vscode-extension/language-configuration.json /tmp/vsix_pack/extension/
cp -r vscode-extension/syntaxes /tmp/vsix_pack/extension/
cp -r vscode-extension/snippets /tmp/vsix_pack/extension/
cp -r vscode-extension/dist /tmp/vsix_pack/extension/
cp vscode-extension/README.md /tmp/vsix_pack/extension/
cp LICENSE /tmp/vsix_pack/extension/

cd /tmp/vsix_pack
zip -r /app/applet/i-language-0.1.0.vsix "[Content_Types].xml" extension.vsixmanifest extension > /dev/null
cp /app/applet/i-language-0.1.0.vsix /app/applet/vscode-extension/

echo "==> Successfully packaged: i-language-0.1.0.vsix ($(du -h /app/applet/i-language-0.1.0.vsix | cut -f1))"
