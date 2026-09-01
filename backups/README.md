# Device backup

The original 16MB flash image was read before the first custom upload on
September 1, 2026.

```text
File: stopwatch-factory-2026-09-01.bin
Size: 16777216 bytes
SHA-256: 6588847c2eb5e546e09df6df8a3a0b3d321f40a21a0c307e3cc2c5c8fe8883b0
Device MAC: 28:84:85:43:a0:4c
```

Verify and restore with:

```sh
shasum -a 256 backups/stopwatch-factory-2026-09-01.bin
./scripts/restore_factory.sh /dev/cu.usbmodem1101 \
  backups/stopwatch-factory-2026-09-01.bin
```

The binary image is intentionally ignored by Git.

