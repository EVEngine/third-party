#!/usr/bin/env bash
set -euo pipefail

# CI network recovery: EVEngine/third-party#11, owner: dependency maintainers.
# Stock runner Ubuntu endpoints failed DNS on both x64 and ARM64. Remove the
# alternate transport when that runner issue is resolved; archive signing keys
# and package authentication remain the distribution's originals throughout.
source /etc/os-release
test "$ID" = ubuntu
case "$(dpkg --print-architecture)" in
  amd64) archive=ubuntu; primary=https://archive.ubuntu.com/ubuntu ;;
  arm64) archive=ubuntu-ports; primary=https://ports.ubuntu.com/ubuntu-ports ;;
  *) echo 'Unsupported Ubuntu runner architecture' >&2; exit 1 ;;
esac

setup_dir="${RUNNER_TEMP:-/tmp}/eve-apt"
mkdir -p "$setup_dir"
mirror=''
for candidate in "$primary" "https://mirrors.tuna.tsinghua.edu.cn/$archive"; do
  echo "Checking Ubuntu package transport: $candidate"
  if curl --fail --silent --show-error --location --connect-timeout 10 --max-time 30 \
      "$candidate/dists/$VERSION_CODENAME/InRelease" -o "$setup_dir/InRelease"; then
    mirror="$candidate"
    break
  fi
done
if [ -z "$mirror" ]; then
  echo 'No Ubuntu package transport is reachable' >&2
  exit 1
fi
echo "Installing signed Ubuntu packages from $mirror"
cat > "$setup_dir/ubuntu.sources" <<EOF
Types: deb
URIs: $mirror
Suites: $VERSION_CODENAME $VERSION_CODENAME-updates $VERSION_CODENAME-backports $VERSION_CODENAME-security
Components: main restricted universe multiverse
Signed-By: /usr/share/keyrings/ubuntu-archive-keyring.gpg
EOF

apt_options=(-o "Dir::Etc::sourcelist=$setup_dir/ubuntu.sources" -o 'Dir::Etc::sourceparts=-'
  -o Acquire::Retries=2 -o Acquire::http::Timeout=30 -o Acquire::https::Timeout=30
  -o APT::Update::Error-Mode=any)
# sudo otherwise drops runner-provided proxy configuration. Avoid logging its
# values; apt still verifies HTTPS and the signed Ubuntu archive metadata.
sudo --preserve-env=HTTP_PROXY,HTTPS_PROXY,ALL_PROXY,NO_PROXY,http_proxy,https_proxy,all_proxy,no_proxy \
  env "http_proxy=${http_proxy:-${HTTP_PROXY:-}}" "https_proxy=${https_proxy:-${HTTPS_PROXY:-}}" \
  apt-get "${apt_options[@]}" update
sudo --preserve-env=HTTP_PROXY,HTTPS_PROXY,ALL_PROXY,NO_PROXY,http_proxy,https_proxy,all_proxy,no_proxy \
  env "http_proxy=${http_proxy:-${HTTP_PROXY:-}}" "https_proxy=${https_proxy:-${HTTPS_PROXY:-}}" \
  apt-get "${apt_options[@]}" install -y "$@"
