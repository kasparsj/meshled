import { useEffect, useMemo, useRef, useState } from 'react'

const CHANNELS = {
  stable: {
    id: 'stable',
    title: 'Stable',
    description: 'Recommended for production installs and long-running pieces.',
    manifest: 'firmware/manifest-stable.json',
    releaseNotesUrl: 'https://github.com/kasparsj/meshled/releases',
  },
  beta: {
    id: 'beta',
    title: 'Beta',
    description: 'Preview builds with newer features and higher regression risk.',
    manifest: 'firmware/manifest-beta.json',
    releaseNotesUrl: 'https://github.com/kasparsj/meshled/releases',
  },
}

function StatusPill({ label, ok }) {
  return (
    <span className={`status-pill ${ok ? 'status-ok' : 'status-warn'}`}>
      <span className="status-dot" aria-hidden="true" />
      {label}
    </span>
  )
}

export default function App() {
  const [channel, setChannel] = useState('stable')
  const [eraseFirst, setEraseFirst] = useState(false)
  const [manifestLoading, setManifestLoading] = useState(true)
  const [manifestMeta, setManifestMeta] = useState(null)
  const [manifestError, setManifestError] = useState('')
  const installRef = useRef(null)

  const selectedChannel = CHANNELS[channel]
  const secureContext = typeof window !== 'undefined' && window.isSecureContext
  const hasWebSerial = typeof navigator !== 'undefined' && 'serial' in navigator

  useEffect(() => {
    let cancelled = false
    setManifestLoading(true)
    setManifestError('')
    setManifestMeta(null)

    fetch(selectedChannel.manifest, { cache: 'no-store' })
      .then((response) => {
        if (!response.ok) {
          throw new Error(`Manifest HTTP ${response.status}`)
        }

        return response.json()
      })
      .then((manifest) => {
        if (!cancelled) {
          setManifestMeta({
            name: manifest.name,
            version: manifest.version,
            chipFamilies: (manifest.builds || []).map((build) => build.chipFamily),
          })
          setManifestLoading(false)
        }
      })
      .catch((error) => {
        if (!cancelled) {
          setManifestError(`Unable to load firmware manifest: ${error.message}`)
          setManifestLoading(false)
        }
      })

    return () => {
      cancelled = true
    }
  }, [selectedChannel.manifest])

  useEffect(() => {
    const button = installRef.current
    if (!button) {
      return
    }

    button.setAttribute('manifest', selectedChannel.manifest)
    if (eraseFirst) {
      button.setAttribute('erase-first', '')
    } else {
      button.removeAttribute('erase-first')
    }
  }, [eraseFirst, selectedChannel.manifest])

  const canInstall = useMemo(
    () => secureContext && hasWebSerial && !manifestLoading && !manifestError && !!manifestMeta,
    [hasWebSerial, manifestError, manifestLoading, manifestMeta, secureContext],
  )

  return (
    <main className="app-shell">
      <div className="background-glow" aria-hidden="true" />
      <header className="hero-card">
        <p className="eyebrow">MeshLED Firmware Installer</p>
        <h1>Flash MeshLED over USB in your browser</h1>
        <p className="subtitle">
          Similar workflow to install.wled.me: pick your release channel, connect
          your ESP32 board, and flash with Web Serial.
        </p>
        <div className="status-row" role="status" aria-label="Environment checks">
          <StatusPill label={secureContext ? 'Secure context: yes' : 'Secure context: no'} ok={secureContext} />
          <StatusPill label={hasWebSerial ? 'Web Serial: available' : 'Web Serial: unavailable'} ok={hasWebSerial} />
          <StatusPill
            label={
              manifestLoading
                ? 'Manifest: checking'
                : manifestError
                  ? 'Manifest: missing'
                  : 'Manifest: ready'
            }
            ok={!manifestLoading && !manifestError}
          />
        </div>
      </header>

      <section className="channel-grid" aria-label="Release channels">
        {Object.values(CHANNELS).map((entry) => (
          <button
            key={entry.id}
            className={`channel-card ${channel === entry.id ? 'channel-card-active' : ''}`}
            onClick={() => setChannel(entry.id)}
            type="button"
          >
            <span className="channel-title">{entry.title}</span>
            <span className="channel-desc">{entry.description}</span>
          </button>
        ))}
      </section>

      <section className="install-card" aria-label="Installer panel">
        <div className="install-header">
          <h2>Install Firmware</h2>
          <p>Channel: {selectedChannel.title}</p>
        </div>

        {manifestMeta && (
          <p className="manifest-meta">
            {manifestMeta.name} {manifestMeta.version} for {manifestMeta.chipFamilies.join(', ')}
          </p>
        )}

        {manifestError && <p className="inline-error">{manifestError}</p>}

        <label className="erase-toggle">
          <input
            type="checkbox"
            checked={eraseFirst}
            onChange={(event) => setEraseFirst(event.target.checked)}
          />
          Erase flash before install (recommended when migrating from other firmware)
        </label>

        <div className="installer-action">
          <esp-web-install-button ref={installRef} key={selectedChannel.manifest}>
            <button
              slot="activate"
              className="install-button"
              type="button"
              disabled={!canInstall}
            >
              Install MeshLED
            </button>
          </esp-web-install-button>
        </div>

        {!hasWebSerial && (
          <p className="hint-text">
            Use a Chromium browser (Chrome, Edge, Opera, or Brave) on desktop.
          </p>
        )}

        {!secureContext && (
          <p className="hint-text">
            Web Serial requires HTTPS or localhost. Open this installer from a
            secure origin.
          </p>
        )}
      </section>

      <section className="notes-grid" aria-label="Support links">
        <article className="note-card">
          <h3>Release Notes</h3>
          <p>Inspect firmware changes before flashing a production installation.</p>
          <a href={selectedChannel.releaseNotesUrl} target="_blank" rel="noreferrer">
            View MeshLED releases
          </a>
        </article>
        <article className="note-card">
          <h3>Manual Flashing</h3>
          <p>Need serial logs or custom offsets? Use PlatformIO or esptool.</p>
          <a
            href="https://github.com/kasparsj/meshled/blob/main/docs-site/src/content/docs/installer-web.md"
            target="_blank"
            rel="noreferrer"
          >
            Open flashing guide
          </a>
        </article>
      </section>
    </main>
  )
}
