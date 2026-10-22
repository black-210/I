import tailwindcss from '@tailwindcss/vite';
import react from '@vitejs/plugin-react';
import path from 'path';
import { defineConfig, Plugin } from 'vite';
import { exec } from 'child_process';
import fs from 'fs';

function compilerApiPlugin(): Plugin {
  return {
    name: 'i-compiler-api',
    configureServer(server) {
      server.middlewares.use('/api/compiler', (req, res) => {
        if (req.method === 'POST') {
          let body = '';
          req.on('data', chunk => { body += chunk; });
          req.on('end', () => {
            try {
              const data = JSON.parse(body);
              const cmd = data.cmd || 'test';
              const file = data.file || 'examples/beginner/02_binary_and_bits.i';
              const code = data.code;

              let targetPath = file;
              if (code) {
                targetPath = `/tmp/custom_${Date.now()}.i`;
                fs.writeFileSync(targetPath, code);
              }

              let fullCmd = `./bin/i ${cmd} ${targetPath}`;
              if (cmd === 'test') fullCmd = `./bin/i test`;
              else if (cmd === 'targets') fullCmd = `./bin/i targets`;

              exec(fullCmd, { timeout: 10000 }, (error, stdout, stderr) => {
                if (code && fs.existsSync(targetPath)) {
                  fs.unlinkSync(targetPath);
                }
                res.setHeader('Content-Type', 'application/json');
                res.end(JSON.stringify({
                  success: !error || error.code === 0,
                  exitCode: error ? error.code : 0,
                  stdout: stdout || '',
                  stderr: stderr || '',
                  cmd: fullCmd
                }));
              });
            } catch (err: any) {
              res.setHeader('Content-Type', 'application/json');
              res.statusCode = 500;
              res.end(JSON.stringify({ error: err.message }));
            }
          });
        } else {
          res.statusCode = 405;
          res.end('Method Not Allowed');
        }
      });
    }
  };
}

export default defineConfig(() => {
  return {
    plugins: [react(), tailwindcss(), compilerApiPlugin()],
    resolve: {
      alias: {
        '@': path.resolve(__dirname, '.'),
      },
    },
    server: {
      hmr: process.env.DISABLE_HMR !== 'true',
      watch: process.env.DISABLE_HMR === 'true' ? null : {},
    },
  };
});
