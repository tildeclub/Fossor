# CI and release builds

GitHub Actions replaces the old Travis setup.

- **CI:** runs for pushes to `master`, pull requests and manual dispatch. It
  builds with GCC and Clang on Ubuntu 24.04 and 26.04 (x86-64), GCC on Ubuntu
  26.04 ARM64, and Clang on macOS 15 and 26 (Intel and Apple Silicon). The
  smoke test runs without installing the server or requiring root.
- **Release binaries:** runs when a GitHub Release is **published**, including
  a published prerelease. It checks out the release tag, builds on four native
  runners, runs the smoke tests, and attaches the resulting `.tar.gz` archives
  plus `SHA256SUMS` to that existing release. If a build fails, nothing is
  uploaded by the publishing job.

## Publishing

1. Push the changes containing these workflows to `master` **before** making
   a release. Ensure the release tag points to a commit containing them.
2. Create a tag such as `v3.1.2` (or `v3.1.2-rc1`) and publish the GitHub
   Release using that tag. Draft releases do not trigger the workflow until
   published.
3. The `Release binaries` workflow attaches four archives:
   Linux x86-64, Linux ARM64, macOS Intel and macOS Apple Silicon. GitHub
   supplies its standard source archives separately.

Release binaries intentionally stay on Ubuntu 24.04 (x86-64 and ARM64) and
macOS 15 (Intel and Apple Silicon) to avoid unnecessarily raising the minimum
runtime requirements. Newer operating systems are tested by CI instead.

The binary version is derived from the release tag **in the build workspace**;
the workflow does not commit changes to the repository. Keep the version in
`Makefile.in` up to date in normal development as well. These are dynamically
linked binaries from GitHub's runners, not static binaries or installable OS
packages. The macOS binaries are not signed or notarized.

The release workflow uses the automatically provided `GITHUB_TOKEN` with
`contents: write` permission for its publishing job only. No PAT or separate
secret is needed. You can rerun failed jobs under **Actions** if a build fails.

To run the smoke tests locally after `./configure` and `make`:

```sh
sh tests/smoke.sh
```
