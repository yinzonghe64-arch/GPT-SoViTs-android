
#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import json
from pathlib import Path


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--upstream", type=Path, default=Path("../GPT-SoVITS"))
    parser.add_argument("--output", type=Path, default=Path("build/japanese.json"))
    args = parser.parse_args()

    source = args.upstream.resolve() / "GPT_SoVITS/text/symbols2.py"

    if not source.is_file():
        raise SystemExit(f"Missing upstream symbols file: {source}")

    spec = importlib.util.spec_from_file_location("gsv_symbols2", source)
    if spec is None or spec.loader is None:
        raise SystemExit("Cannot load symbols2.py")

    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    symbols = module.symbols
    japanese = module.ja_symbols

    if len(symbols) != len(set(symbols)):
        raise SystemExit("Duplicate symbols detected")

    if "[" not in symbols or "]" not in symbols:
        raise SystemExit("Missing Japanese prosody symbols")

    symbol_ids = {
        symbol: index
        for index, symbol in enumerate(symbols)
    }

    required = set(japanese)
    required.update(["[", "]", "UNK", "!", "?", "…", ",", ".", "-", "_"])

    missing = sorted(required - set(symbol_ids))
    if missing:
        raise SystemExit(f"Missing required symbols: {missing}")

    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)

    result = {
        "format": "gsv-mobile-japanese-frontend",
        "version": 1,
        "profile": "v2ProPlus",
        "source": "GPT_SoVITS/text/symbols2.py",
        "source_sha256": sha256(source),
        "symbol_count": len(symbols),
        "japanese_symbols": japanese,
        "symbol_ids": symbol_ids,
        "special_phone_fallback": "UNK"
    }

    temporary = output.with_suffix(output.suffix + ".partial")
    temporary.write_text(
        json.dumps(result, ensure_ascii=False, indent=2),
        encoding="utf-8"
    )
    temporary.replace(output)

    print(f"Created: {output}")
    print(f"Total symbols: {len(symbols)}")
    print(f"Japanese symbols: {len(japanese)}")
    print(f"SHA256: {result['source_sha256']}")


if __name__ == "__main__":
    main()

