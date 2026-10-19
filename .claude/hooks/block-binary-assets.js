// Copyright © 2026 Helen Allien Poe. See LICENSE.
// PreToolUse hook: refuse Edit/Write on Unreal binary assets. Exit 2 blocks the call and shows stderr to Claude.
let input = '';
process.stdin.on('data', chunk => (input += chunk));
process.stdin.on('end', () => {
  const path = (JSON.parse(input).tool_input || {}).file_path || '';
  if (/\.(uasset|umap)$/i.test(path)) {
    process.stderr.write(`${path} is a binary Unreal asset — change it in the editor or via a Tools/Python script, never by text edit.`);
    process.exit(2);
  }
});
