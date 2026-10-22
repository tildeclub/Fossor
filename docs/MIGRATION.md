# Moving from Gophernicus to Fossor

Fossor retains Gophernicus's Gopher protocol features, gophermap syntax,
executable maps, `=` shell includes, CGI scripts, personal gopherspaces,
virtual hosting, and command-line options. The project files and service
names have changed; an existing installation is not replaced automatically.

## What changed

| Old name | New name |
| --- | --- |
| `gophernicus` binary | `fossor` |
| `gophernicus.8` manual | `fossor.8` |
| `gophernicus.socket` | `fossor.socket` |
| `gophernicus@.service` | `fossor@.service` |
| `/etc/default/gophernicus` | `/etc/default/fossor` |
| `/etc/sysconfig/gophernicus` | `/etc/sysconfig/fossor` |
| `org.gophernicus.server` (macOS) | `org.fossor.server` |

The default Gopher port (70), gopher root (`/var/gopher`), personal
`public_gopher` directory, `gophermap` and `gophertag` names remain unchanged.
You do not need to rename users' content or change CGI script interfaces.
Existing explicit binary paths in your own scripts must be updated if used.

## On an existing systemd host

1. Back up your existing configuration and note the current arguments.
2. Install Fossor, without starting its socket. Do **not** run both sockets
   on TCP port 70 at the same time.
3. **After installing**, copy the old environment file to the matching Fossor
   location. `make install` writes a default file, so copying before install
   could lose your settings. The generated units read `/etc/default/fossor`
   and `/etc/sysconfig/fossor` (as configured at build time). Preserve
   `OPTIONS` including `-h`, `-r`, proxy settings and any custom filters.
4. Switch listeners once configuration is in place:

   ```sh
   sudo systemctl disable --now gophernicus.socket
   sudo systemctl daemon-reload
   sudo systemctl enable --now fossor.socket
   sudo systemctl status fossor.socket
   ```

The old socket and binary are not deleted by this change. Remove the previous
package or unit manually after checking the new service, or keep them for a
controlled rollback. If the original service was named differently, use the
name actually installed on your host. For inetd/xinetd, replace the old daemon
path and avoid duplicate definitions listening on the same port.

On macOS, unload the old `org.gophernicus.server` launchd service before
loading `org.fossor.server`.

## Other changes

- Fossor uses a separate System V shared-memory key so it cannot accidentally
  reuse the old daemon's session state. Live request counters start fresh.
- The release version is `3.1.2`; GitHub Actions packages the `fossor` binary.
- Existing site symlinks, executable gophermaps, CGI and shell includes are
  preserved. These are not rewritten automatically.
- Filter directories such as `/usr/lib/gophernicus/filters` can still be used
  by passing the original path with `-f`. The new documentation uses a
  Fossor-named example directory but does not require you to move filters.

After migrating, test a static file, a directory menu, an executable gophermap
and a CGI endpoint before removing the old service.
