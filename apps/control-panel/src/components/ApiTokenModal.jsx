import React, { useEffect, useMemo, useState } from 'react';
import { KeyRound, X, Eye, EyeOff, Trash2 } from 'lucide-react';

const ApiTokenModal = ({
    isOpen,
    onClose,
    onSave,
    onClear,
    currentToken = '',
    authError = '',
    requireToken = false,
}) => {
    const [token, setToken] = useState(currentToken);
    const [showToken, setShowToken] = useState(false);
    const [error, setError] = useState('');

    useEffect(() => {
        if (isOpen) {
            setToken(currentToken || '');
            setError('');
            setShowToken(false);
        }
    }, [isOpen, currentToken]);

    const blocking = requireToken && String(currentToken || '').trim().length === 0;

    const description = useMemo(() => {
        if (blocking) {
            return 'This device requires an API token before protected routes can be used.';
        }
        return 'Store a bearer token for protected firmware routes.';
    }, [blocking]);

    if (!isOpen) return null;

    const handleSave = () => {
        const trimmed = token.trim();
        if (trimmed.length === 0) {
            setError('Token cannot be empty.');
            return;
        }
        onSave(trimmed);
        if (!blocking) {
            onClose();
        }
    };

    const handleClear = () => {
        onClear();
        setToken('');
        setError('');
    };

    return (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
            <div className="bg-zinc-800 rounded-lg p-6 w-full max-w-lg mx-4">
                <div className="flex items-center justify-between mb-4">
                    <h3 className="text-lg font-semibold flex items-center gap-2">
                        <KeyRound size={20} />
                        API Token
                    </h3>
                    <button
                        onClick={onClose}
                        disabled={blocking}
                        className="text-zinc-400 hover:text-white disabled:opacity-50"
                    >
                        <X size={20} />
                    </button>
                </div>

                <p className="text-sm text-zinc-300 mb-4">{description}</p>

                <div className="space-y-3">
                    <div className="relative">
                        <input
                            type={showToken ? 'text' : 'password'}
                            value={token}
                            onChange={(event) => {
                                setToken(event.target.value);
                                if (error) {
                                    setError('');
                                }
                            }}
                            className="w-full bg-zinc-700 border border-zinc-600 rounded px-3 py-2 pr-10 text-white"
                            placeholder="Enter bearer token"
                        />
                        <button
                            type="button"
                            className="absolute right-2 top-1/2 -translate-y-1/2 text-zinc-400 hover:text-white"
                            onClick={() => setShowToken((prev) => !prev)}
                        >
                            {showToken ? <EyeOff size={16} /> : <Eye size={16} />}
                        </button>
                    </div>

                    {(error || authError) && (
                        <div className="bg-red-900/50 border border-red-500 text-red-200 px-3 py-2 rounded text-sm">
                            {error || authError}
                        </div>
                    )}
                </div>

                <div className="mt-6 flex justify-end gap-3">
                    <button
                        onClick={handleClear}
                        className="px-4 py-2 rounded bg-zinc-700 hover:bg-zinc-600 text-zinc-100 flex items-center gap-2"
                    >
                        <Trash2 size={16} />
                        Clear
                    </button>
                    <button
                        onClick={handleSave}
                        className="px-5 py-2 rounded bg-sky-600 hover:bg-sky-700 text-white"
                    >
                        Save Token
                    </button>
                </div>
            </div>
        </div>
    );
};

export default ApiTokenModal;
