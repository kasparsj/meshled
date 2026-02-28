import React, { createContext, useCallback, useContext, useMemo, useState } from 'react';

const ToastContext = createContext();

let nextToastId = 1;

const ToastItem = ({ toast, onDismiss }) => {
    const palette = {
        success: 'bg-green-700 border-green-500',
        error: 'bg-red-700 border-red-500',
        info: 'bg-sky-700 border-sky-500',
    };

    return (
        <div className={`border px-3 py-2 rounded shadow-lg text-sm ${palette[toast.type] || palette.info}`}>
            <div className="flex items-start justify-between gap-3">
                <p className="text-white">{toast.message}</p>
                <button
                    className="text-white/70 hover:text-white"
                    onClick={() => onDismiss(toast.id)}
                    aria-label="Dismiss notification"
                >
                    ×
                </button>
            </div>
        </div>
    );
};

export const ToastProvider = ({ children }) => {
    const [toasts, setToasts] = useState([]);

    const dismissToast = useCallback((toastId) => {
        setToasts((prev) => prev.filter((toast) => toast.id !== toastId));
    }, []);

    const showToast = useCallback((message, type = 'info', durationMs = 3200) => {
        const id = nextToastId++;
        setToasts((prev) => [...prev, { id, message, type }]);

        if (durationMs > 0) {
            window.setTimeout(() => {
                setToasts((prev) => prev.filter((toast) => toast.id !== id));
            }, durationMs);
        }

        return id;
    }, []);

    const value = useMemo(() => ({ showToast, dismissToast }), [showToast, dismissToast]);

    return (
        <ToastContext.Provider value={value}>
            {children}
            <div className="fixed top-4 right-4 z-[100] space-y-2 w-[320px] max-w-[calc(100vw-2rem)]">
                {toasts.map((toast) => (
                    <ToastItem key={toast.id} toast={toast} onDismiss={dismissToast} />
                ))}
            </div>
        </ToastContext.Provider>
    );
};

export const useToast = () => {
    const context = useContext(ToastContext);
    if (!context) {
        throw new Error('useToast must be used within a ToastProvider');
    }
    return context;
};
