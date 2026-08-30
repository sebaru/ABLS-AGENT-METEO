# abls-agent-meteo

Standalone meteo runtime for Abls-Habitat.

## Current implementation status

- Runtime based on `abls-agent-libs`
- Keeps SRC `Watchdogd/Meteo` business logic:
  - ephemeride and daily forecast retrieved from the Météo-Concept API
  - `SUNRISE` / `SUNSET` HORLOGE ticks updated from the ephemeride
  - 12 AI per day over 14 days (`DAY<n>_TEMP_MIN`, `DAY<n>_WEATHER`, ...) published through MQTT
  - adaptive polling: hourly when the site answers, every minute otherwise
  - communication status reporting to master (`IO_COMM`)

Expected API config fields:

- `token` (Météo-Concept API token)
- `code_insee` (INSEE code of the city)

Both can also be provided locally, through `/etc/abls-agent.conf`, environment
(`ABLS_TOKEN`, `ABLS_CODE_INSEE`) or command line (`--token=`, `--code-insee=`).

## Build

```sh
./install_deps.sh
./build.sh
```

## Packaging RPM

```sh
./build_rpm.sh
```

Produces runtime RPM package in `build/`.

The runtime package also installs a templated systemd unit:

- `abls-agent-meteo@.service`

Start one instance per agent tech id:

```sh
sudo systemctl enable --now abls-agent-meteo@<agent-tech-id>.service
```

## Packaging DEB

```sh
./build_apt.sh --dist bookworm
./build_apt.sh --dist trixie
```

Default target suite is detected from host OS codename (`/etc/os-release`), with `bookworm` fallback.

Produces runtime DEB package and copies normalized artifacts to:

- `build/pkgs/deb/<suite>/<arch>/`

`build_apt.sh` builds only the native host architecture.

Package signatures are centralized in ABLS-PKGS (both DEB repository metadata and RPM package/repository signatures).

The DEB package installs the same templated systemd unit:

```sh
sudo systemctl enable --now abls-agent-meteo@<agent-tech-id>.service
```

## Release bump + publication

```sh
./bump.sh 1.2.3
```

The release flow:

- tags `v1.2.3` from `trunk`
- merges `trunk` into `main`
- builds RPM + DEB packages
- copies RPM to `../ABLS-PKGS/public/rpms/<arch>/`

## Container build

```sh
podman build -t abls-agent-meteo:dev \
  --build-arg ABLS_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_DEVEL_RPM_URL=<url> \
  --build-arg ABLS_LIBS_RPM_URL=<url> \
  --build-arg ABLS_AGENT_LIBS_RPM_URL=<url> \
  -f Containerfile .
```
