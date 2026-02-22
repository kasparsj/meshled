import {Zap} from "lucide-react";
import React from "react";

const EmitterTab = () => (
    <div className="space-y-6">
        <h2 className="text-2xl font-bold">Emitter Settings</h2>

        <div className="bg-zinc-700 p-4 rounded-lg">
            <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
                <div>
                    <h3 className="text-lg font-semibold mb-4">Speed Range</h3>
                    <div className="space-y-4">
                        <div>
                            <label className="block text-sm text-zinc-300 mb-2">Min Speed</label>
                            <input type="range" min="0" max="255" className="w-full" />
                        </div>
                        <div>
                            <label className="block text-sm text-zinc-300 mb-2">Max Speed</label>
                            <input type="range" min="0" max="255" className="w-full" />
                        </div>
                    </div>
                </div>

                <div>
                    <h3 className="text-lg font-semibold mb-4">Duration Range</h3>
                    <div className="space-y-4">
                        <div>
                            <label className="block text-sm text-zinc-300 mb-2">Min Duration (ms)</label>
                            <input type="number" min="0" className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2" />
                        </div>
                        <div>
                            <label className="block text-sm text-zinc-300 mb-2">Max Duration (ms)</label>
                            <input type="number" min="0" className="w-full bg-zinc-600 border border-zinc-500 rounded px-3 py-2" />
                        </div>
                    </div>
                </div>
            </div>

            <div className="mt-6">
                <button className="bg-orange-600 hover:bg-orange-700 px-4 py-2 rounded-lg flex items-center gap-2">
                    <Zap size={18} />
                    Toggle Auto Mode
                </button>
            </div>
        </div>
    </div>
);

export default EmitterTab;