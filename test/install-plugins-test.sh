#!/usr/bin/env bash
# Exercise the real installer against fake bundles; no plugins or AU caches are touched.
set -euo pipefail

source_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
fixture="$(mktemp -d)"
trap 'rm -rf "$fixture"' EXIT
mkdir -p "$fixture/scripts" "$fixture/bin"
cp "$source_root/scripts/install-plugins.sh" "$fixture/scripts/"
export ISO3D_INSTALL_LOG="$fixture/operations.log"

# Intercept every command that could change plugin directories or cache state.
for command in mkdir cp rm codesign killall; do
    cat > "$fixture/bin/$command" <<'MOCK'
#!/usr/bin/env bash
printf '%s %s\n' "${0##*/}" "$*" >> "$ISO3D_INSTALL_LOG"
MOCK
    chmod +x "$fixture/bin/$command"
done

run_installer() {
    PATH="$fixture/bin:$PATH" bash "$fixture/scripts/install-plugins.sh" "$@"
}

if run_installer > "$fixture/output" 2>&1; then
    echo "Expected failure when no bundles exist" >&2
    exit 1
fi
[[ ! -e "$ISO3D_INSTALL_LOG" ]] || { echo "Missing builds must not touch plugins or caches" >&2; exit 1; }

# The default preset produces Debug; the release switch selects release-build/Release.
for configuration in Debug Release; do
    build_dir=build
    args=()
    if [[ "$configuration" == Release ]]; then
        build_dir=release-build
        args=(--release)
    fi
    bundle="$fixture/$build_dir/plugin/AudioPlugin_artefacts/$configuration"
    mkdir -p "$bundle/AU/Iso3D.component" "$bundle/VST3/Iso3D.vst3"
    : > "$ISO3D_INSTALL_LOG"
    run_installer "${args[@]}" > "$fixture/output"
    operations="$(cat "$ISO3D_INSTALL_LOG")"
    [[ "$operations" == *"cp -R $bundle/AU/Iso3D.component"* ]] || exit 1
    [[ "$operations" == *"cp -R $bundle/VST3/Iso3D.vst3"* ]] || exit 1
    [[ "$operations" == *"codesign --force --sign -"* ]] || exit 1
    echo "PASS: $configuration installer selects and signs both expected bundles"
done

: > "$ISO3D_INSTALL_LOG"
if run_installer --sign-identity > "$fixture/output" 2>&1; then
    echo "Expected failure when signing identity is missing" >&2
    exit 1
fi
[[ ! -s "$ISO3D_INSTALL_LOG" ]] || exit 1
echo "PASS: missing builds and missing signing identity fail without side effects"
