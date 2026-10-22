#!/bin/sh
# Small regression test for the shipped server, without installing it.
set -eu
cd "$(dirname "$0")/.."

tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT HUP INT TERM
chmod 755 "$tmpdir"
cp tests/test.gophermap "$tmpdir/gophermap"
chmod 644 "$tmpdir/gophermap"

# Keep the original Travis fixture and expected output (including CRLF).
printf '\r\n' | ./src/gophernicus -r "$tmpdir" -h localhost \
    -nf -nu -nv -nx -nd -nm -ns -Q 10 > "$tmpdir/output"
cmp tests/test.answer "$tmpdir/output"

./src/gophernicus -v | grep -q '^Gophernicus/'

# Check that executable gophermaps still work.
cat > "$tmpdir/gophermap" <<'MAP'
#!/bin/sh
printf 'Dynamic\n'
MAP
chmod 755 "$tmpdir/gophermap"
printf '\r\n' | ./src/gophernicus -r "$tmpdir" -h localhost \
    -nf -nu -nv -nd -nm -ns -Q 10 > "$tmpdir/output"
grep -q '^iDynamic' "$tmpdir/output"

# A gophermap '=' entry is intentionally allowed to run a shell command.
cat > "$tmpdir/gophermap" <<'MAP'
=printf 'Shell include\n'
MAP
chmod 644 "$tmpdir/gophermap"
printf '\r\n' | ./src/gophernicus -r "$tmpdir" -h localhost \
    -nf -nu -nv -nd -nm -ns -Q 10 > "$tmpdir/output"
grep -q '^iShell include' "$tmpdir/output"

echo 'Smoke tests passed'
