#!/usr/bin/env python2
# -*- coding: utf-8 -*-

import argparse, sys, re, math, os

def get_tree_list(input_file):
	tree_list = [key.GetName() for key in input_file.GetListOfKeys() if key.GetClassName() == 'TTree']
	return tree_list

def check_for_tree(input_file, input_tree_name):
	tree_list = get_tree_list(input_file)
	if len(tree_list) == 0:
		print('No tree in %s found!'%(input_file_name))
		sys.exit(1)
	
	if len(tree_list) > 1 and not input_tree_name:
		print('Multiple trees found: %s. Provide -t/--tree!'%(', '.join(tree_list)))
		sys.exit(1)

	if len(tree_list) == 1 and not input_tree_name:
		input_tree_name = tree_list[0]
		print('Choosing tree %s to split.'%(input_tree_name))

	if input_tree_name not in tree_list:
		print('Could not find tree %s in %s'%(input_file_name, input_file_name))
		sys.exit(1)

def splitFile(args):
	from ROOT import TFile, TTree # here, otherwise -h/--help will not work

	if (not args.nevents and not args.njobs) or (args.nevents and args.njobs):
		print('Give either -j/--njobs or -n/--nevents!')
		sys.exit(1)

	if not args.batch:
		if args.nevents:
			print('Splitting tree %s of file %s into parts of %d events each.'%(args.tree, args.inputFileName, args.nevents))
		else:
			print('Splitting tree %s of file %s into %d parts.'%(args.tree, args.inputFileName, args.njobs))

	input_file = TFile(args.inputFileName, 'read')
	check_for_tree(input_file, args.tree)
	input_tree = input_file.Get(args.tree)

	output_file_names = []
	nevents = input_tree.GetEntries()

	if args.njobs:
		args.nevents = int(math.ceil(float(nevents) / args.njobs))

	for file_num, event_nums in enumerate([range(nevents)[i:i+args.nevents] for i in range(0, nevents, args.nevents)]):
		input_file_base, extension = os.path.splitext(args.inputFileName)
		output_file_name = '%s_%s%s'%(input_file_base, file_num, extension)
		output_file_names.append(output_file_name)
		output_file = TFile(output_file_name, 'recreate')
		output_tree = input_tree.CloneTree(0);

		for event_num in event_nums:
			input_tree.GetEntry(event_num)
			output_tree.Fill()
		output_file.Write()
		output_file.Close()

	if not args.batch:
		print('Wrote: %s'%(', '.join(output_file_names)))
	else:
		for output_file_name in output_file_names: print(output_file_name)

def main():
	parser = argparse.ArgumentParser(description='Split ROOT in multiple files.')
	parser.add_argument('inputFileName', help='ROOT input file to split')
	parser.add_argument('-n', '--nevents', help='Number of events per file', type=int)
	parser.add_argument('-j', '--njobs', help='Number of jobs to split up to', type=int)
	parser.add_argument('-t', '--tree', help='TTree to split')
	parser.add_argument('-b', '--batch', help='Run in batch mode, only outputting the resulting files', action='store_true')

	args = parser.parse_args()
	splitFile(args)

if __name__ == '__main__':
	main()
