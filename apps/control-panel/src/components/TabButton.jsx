import React from "react";

const TabButton = ({ id, icon, label, active, onClick }) => {
    const Icon = icon;

    return (
        <button
            onClick={() => onClick(id)}
            className={`flex items-center gap-2 px-4 py-2 rounded-lg transition-all ${
                active
                    ? 'bg-sky-600 text-white shadow-lg'
                    : 'bg-zinc-700 text-zinc-300 hover:bg-zinc-600 hover:text-white'
            }`}
        >
            <Icon size={18} />
            {label}
        </button>
    );
};

TabButton.propTypes = {

};

export default TabButton;
