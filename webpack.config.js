const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;
const path = require('path');
const fs = require('fs-extra');
let opt_dist = 'dist';
let opt_watch = false;
let inputs = { index: './src/index.ts' };
module.exports = (env) => {
	if (process.env.opt == 'dev') {
		opt_watch = true;
		inputs.test = './test/index.ts';
	}
	const nodeConfig = {
		// Change to your "entry-point".
		watch: opt_watch,
		entry: inputs,
		output: {
			path: path.resolve(__dirname, opt_dist),
			filename: '[name].js',
			libraryTarget: 'commonjs2'
		},
		resolve: {
			extensions: [ '.ts', '.js', '.json' ]
		},
		module: {
			rules: [
				{
					// Include ts, tsx, js, and jsx files.
					test: /\.(js)x?$/,
					loader: 'babel-loader'
				}
			],
			rules: [
				{
					// Include ts, tsx, js, and jsx files.
					test: /\.(ts)x?$/,
					loader: 'ts-loader',
					options: {
						// 指定特定的ts编译配置，为了区分脚本的ts配置
						configFile: path.resolve(__dirname, './tsconfig.json')
					}
				}
			]
		},
		node: {
			// Replace these Node.js native modules with empty objects, Mongoose's
			// browser library does not use them.
			// See https://webpack.js.org/configuration/node/
			fs: false,
			util: false,
			events: false,
			http: false,
			dgram: false,
			net: false,
			buffer: false,
			crypto: false,
			dns: false,
			assert: false,
			stream: false,
			os: false,
			url: false,
			querystring: false,
			tls: false,
			path: false,
			tty: false,
			module: false,
			zlib: false,
			__dirname: false,
			__filename: false
		},
		//plugins: [ new BundleAnalyzerPlugin() ],
		externals: [ './sqlite.passoa' ],
		devtool: 'source-map',
		target: 'node',
		mode: 'development' //'production'
	};
	return [ nodeConfig ];
};
