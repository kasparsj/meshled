const PROXY_TEMPLATE = String(import.meta.env.VITE_DEVICE_PROXY_TEMPLATE || '').trim();

const stripProtocolAndPath = (value) => {
    return String(value || '')
        .trim()
        .replace(/^https?:\/\//i, '')
        .replace(/^\[|\]$/g, '')
        .replace(/\/.*$/, '');
};

const normalizeHostForCompare = (value) => {
    const raw = stripProtocolAndPath(value).toLowerCase();
    if (!raw) {
        return '';
    }

    const ipv6Like = raw.includes(':') && !raw.includes('.');
    if (!ipv6Like || raw.startsWith('[')) {
        return raw;
    }

    return raw;
};

const appendPath = (base, path) => {
    if (!path.startsWith('/')) {
        return `${base}/${path}`;
    }
    return `${base}${path}`;
};

export const sanitizeHost = (value) => stripProtocolAndPath(value);

export const isSameHostAsCurrentPage = (host) => {
    if (typeof window === 'undefined') {
        return false;
    }

    const targetHost = normalizeHostForCompare(host);
    if (!targetHost) {
        return false;
    }

    const currentHost = normalizeHostForCompare(window.location.host);
    const currentHostname = normalizeHostForCompare(window.location.hostname);
    return targetHost === currentHost || targetHost === currentHostname;
};

export const buildDeviceRequestUrl = (host, path = '/') => {
    const cleanHost = sanitizeHost(host);
    if (!cleanHost) {
        throw new Error('Invalid device host');
    }

    const cleanPath = path.startsWith('/') ? path : `/${path}`;

    if (isSameHostAsCurrentPage(cleanHost)) {
        return cleanPath;
    }

    if (PROXY_TEMPLATE) {
        const templateWithHost = PROXY_TEMPLATE.replaceAll('{host}', encodeURIComponent(cleanHost));
        if (templateWithHost.includes('{path}')) {
            return templateWithHost.replaceAll('{path}', cleanPath);
        }
        return appendPath(templateWithHost.replace(/\/$/, ''), cleanPath);
    }

    if (typeof window !== 'undefined' && window.location.protocol === 'https:') {
        throw new Error(
            'Cannot call http device endpoints from an https page. Open the dashboard on the device IP or configure VITE_DEVICE_PROXY_TEMPLATE.',
        );
    }

    return `http://${cleanHost}${cleanPath}`;
};

const withAuthHeader = (headersLike, apiToken) => {
    const headers = new Headers(headersLike || {});
    const trimmedToken = String(apiToken || '').trim();
    if (trimmedToken.length > 0) {
        headers.set('Authorization', `Bearer ${trimmedToken}`);
    }
    return headers;
};

export const fetchDeviceJsonWithTimeout = async (
    host,
    path,
    { timeoutMs = 2000, apiToken = '', ...fetchOptions } = {},
) => {
    const controller = new AbortController();
    const timeoutId = window.setTimeout(() => controller.abort(), timeoutMs);

    try {
        const response = await fetch(buildDeviceRequestUrl(host, path), {
            ...fetchOptions,
            signal: controller.signal,
            headers: withAuthHeader(fetchOptions.headers, apiToken),
        });

        if (!response.ok) {
            throw new Error(`HTTP ${response.status}`);
        }

        return await response.json();
    } finally {
        window.clearTimeout(timeoutId);
    }
};

export const buildAuthHeaders = (headersLike, apiToken) => withAuthHeader(headersLike, apiToken);
