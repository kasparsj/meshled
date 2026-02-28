// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

const [owner = 'kasparsj', repo = 'meshled'] = (
	process.env.GITHUB_REPOSITORY ?? 'kasparsj/meshled'
).split('/');
const isCI = process.env.GITHUB_ACTIONS === 'true';

export default defineConfig({
	site: `https://${owner}.github.io`,
	base: isCI ? `/${repo}` : '/',
	integrations: [
		starlight({
			title: 'MeshLED Docs',
			description:
				'MeshLED firmware, control panel, installer, and LightGraph integration documentation.',
			customCss: ['./src/styles/custom.css'],
			social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/kasparsj/meshled' }],
			editLink: {
				baseUrl: 'https://github.com/kasparsj/meshled/edit/main/docs-site/',
			},
			sidebar: [
				{
					label: 'Getting Started',
					items: [
						{ label: 'Build and Verification', slug: 'build' },
						{ label: 'Versioning and Release Policy', slug: 'versioning' },
						{ label: 'Installer Workflow', slug: 'installer-web' },
					],
				},
				{
					label: 'For WLED Users',
					items: [{ label: 'Migration Guide', slug: 'wled-users' }],
				},
				{
					label: 'Architecture',
					items: [
						{ label: 'Core Architecture Primer', slug: 'core-architecture' },
						{ label: 'Core Build and Reproducibility', slug: 'core-build' },
						{ label: 'Lightgraph API Inventory', slug: 'lightgraph-api-inventory' },
						{ label: 'Third-Party Licenses', slug: 'third-party-licenses' },
					],
				},
				{
					label: 'Firmware and Runtime Contracts',
					items: [
						{ label: 'Firmware HTTP API', slug: 'firmware-api' },
						{ label: 'OSC Input Contract', slug: 'osc-contract' },
						{ label: 'UI/Firmware Compatibility Matrix', slug: 'ui-firmware-compat' },
					],
				},
			],
		}),
	],
});
