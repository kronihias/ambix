# GitHub Actions: Release Builds & Code Signing

This document explains how to configure the [release workflow](workflows/release.yml) so it can build, sign, notarize and publish ambix VST3 installers for Windows 64-bit and macOS (universal arm64+x86_64).

## Overview

The workflow runs on two triggers:

- **`release: published`** — when you publish a GitHub release, both jobs run automatically and attach the resulting `.pkg` (macOS) and `.exe` (Windows) installers to the release.
- **`workflow_dispatch`** — manual trigger from the *Actions* tab. Lets you pick any branch and choose whether to sign. Outputs are uploaded as workflow artifacts (not attached to a release).

Both jobs are independent and run in parallel (`build-macos` on `macos-14`, `build-windows` on `windows-2022`).

## macOS code signing & notarization

Add the following **repository secrets** in *Settings → Secrets and variables → Actions*:

| Secret | Contents |
|---|---|
| `CODESIGN_CERTIFICATE_P12` | Base64-encoded `.p12` containing both your Developer ID Application and Developer ID Installer certificates (with private keys). |
| `CODESIGN_CERTIFICATE_PASSWORD` | Password for the `.p12` above. |
| `KEYCHAIN_PASSWORD` | Arbitrary password for the temporary CI keychain (any random string). |
| `CODESIGN_APP` | Exact certificate identity string, e.g. `Developer ID Application: Matthias Kronlachner (W52ZCCWU2C)`. |
| `CODESIGN_INSTALLER` | Exact certificate identity string, e.g. `Developer ID Installer: Matthias Kronlachner (W52ZCCWU2C)`. |
| `NOTARIZE_APPLE_ID` | Apple ID email used for notarization. |
| `NOTARIZE_PASSWORD` | App-specific password generated at https://appleid.apple.com (NOT your Apple ID password). |
| `NOTARIZE_TEAM_ID` | Apple Developer Team ID (e.g. `W52ZCCWU2C`). |

### Exporting the `.p12`

In Keychain Access, expand each certificate to confirm the private key is present, then `Cmd-click` *both* certificate rows (Application + Installer), right-click → *Export 2 items…* → format **Personal Information Exchange (.p12)** → set a strong password.

Then base64-encode it for GitHub:

```bash
base64 -i certificate.p12 | pbcopy
```

Paste into the `CODESIGN_CERTIFICATE_P12` secret.

You can find the exact identity strings with:
```bash
security find-identity -v -p codesigning
```

## Windows code signing

| Secret | Contents |
|---|---|
| `WINDOWS_CODESIGN_PFX` | Base64-encoded `.p12` / `.pfx` containing your code-signing certificate for plugin binaries (corresponds to the local `scripts/DevIDApplication.p12`). |
| `WINDOWS_CODESIGN_PFX_PASSWORD` | Password for the certificate above. |
| `WINDOWS_INSTALLER_PFX` | Base64-encoded `.p12` / `.pfx` for signing the NSIS installer (corresponds to the local `scripts/DevIDInstaller.p12`). |
| `WINDOWS_INSTALLER_PFX_PASSWORD` | Password for the installer certificate. |

### Encoding the `.pfx`

```bash
base64 -i scripts/DevIDApplication.p12 | pbcopy   # then paste into WINDOWS_CODESIGN_PFX
base64 -i scripts/DevIDInstaller.p12  | pbcopy    # then paste into WINDOWS_INSTALLER_PFX
```

(If you only have a single certificate for both plugin and installer signing, supply the same value to both secrets.)

## How the workflow handles missing secrets

| Secret state | Result |
|---|---|
| Signing requested (`sign=true` or release event) but `CODESIGN_CERTIFICATE_P12` (macOS) / `WINDOWS_CODESIGN_PFX` (Windows) missing | Workflow logs a warning and falls back to an unsigned build. |
| `workflow_dispatch` with `sign=false` | Always produces unsigned installers — useful for verifying the pipeline without burning notarization quota. |

Unsigned macOS installers will trip Gatekeeper on user machines and unsigned Windows installers will get a SmartScreen warning, so always sign for actual releases.

## Publishing a release

1. Bump the [`VERSION`](../VERSION) file (single line, no `v` prefix, e.g. `0.4.2`).
2. Commit and push to `master`.
3. Create a tag and GitHub release:
   ```bash
   gh release create v0.4.2 --generate-notes
   ```
   The workflow triggers on **publish**, so make sure you don't leave the release as a draft.
4. Watch the *Actions* tab. Both jobs typically take 20–40 minutes (most of macOS time is notarization wait).
5. The signed `.pkg` and `.exe` installers appear as release assets when both jobs finish.

## Doing a dry run

To verify the pipeline on a feature branch without using your signing certificates:

1. Push your branch.
2. Go to *Actions* → *Release* → *Run workflow*.
3. Pick the branch, set `sign` to `false`.
4. Wait for the run; download the workflow artifacts to inspect the unsigned installers.

## Local equivalents

The workflow shells out to the existing scripts so anything that builds in CI also builds locally:

- macOS: `./scripts/build_osx.sh --vst3` (signed, requires `scripts/codesign.env`) or `./scripts/build_osx.sh --vst3 --no-sign` (unsigned dry run).
- Windows: edit/create `scripts/codesign.env` if needed (Windows uses env vars `SIGN_PASS` / `INSTALLER_PASS` instead), then `cd scripts && build_all_win64.bat vst3 sign` (or `vst3` for unsigned).

## Security notes

- Never commit `.p12` / `.pfx` files or `scripts/codesign.env` — `.gitignore` already excludes them.
- App-specific passwords can be revoked at https://appleid.apple.com if leaked.
- The temporary CI keychain is destroyed when the runner VM is recycled.
- Use a dedicated Apple ID for CI if you want to compartmentalize notarization access.
