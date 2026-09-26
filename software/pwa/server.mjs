import { existsSync, readFileSync } from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { startSourceExampleServer } from "arcane-os";

const root = path.dirname(fileURLToPath(import.meta.url));
const sdkRoot = path.join(root, "node_modules", "arcane-os");

const requestedPort = Number.parseInt(process.argv[2] || "4173", 10);
const port = Number.isFinite(requestedPort) ? requestedPort : 4173;

const host = process.argv[3] || "0.0.0.0";
  
const tlsDirectory = process.env.ARCANE_WASM_TLS_ROOT
  ? path.resolve(process.env.ARCANE_WASM_TLS_ROOT)
  : path.join(root, "tls");

const tlsKeyPath = path.join(tlsDirectory, "server-key.pem");
const tlsCertificatePath = path.join(tlsDirectory, "server.pem");
const tls = existsSync(tlsKeyPath) && existsSync(tlsCertificatePath)
  ? {
      key: readFileSync(tlsKeyPath),
      cert: readFileSync(tlsCertificatePath),
    }
  : undefined;

const running = await startSourceExampleServer({
  crossOriginIsolated: true,
  host,
  mounts: [
    {
        root,
        urlPath: "/",
        index: "index.html",
        include: [
            "index.html",
            "components/",
            "modules/",
            "node_modules/",
        ],
    },
    {
      root: path.join(sdkRoot, "src"),
      urlPath: "/src",
    },
    {
      root: path.join(sdkRoot, "browser-runtime"),
      urlPath: "/browser-runtime",
    },
    {
      include: ["arcane/", "strong-type/"],
      root: path.join(sdkRoot, "runtime"),
      urlPath: "/runtime",
    },
  ],
  port,
  startPath: "/",
  tls,
});

console.log(`OpenSync PWA: ${running.url}`);
console.log(`Live Arcane SDK source: ${sdkRoot}`);

await running.closed;