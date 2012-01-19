include Enumerable
#require 'methods/read_initial_structure.rb'
require 'methods/test.rb'

#変数定義

structure_file = "test-first-3bilayers.xyz"

line=Array.new(100) #問題のサイズで変える 

iteration = 1000

Atom = Struct.new(:number, :species, :x, :y, :z)
arr_initial_atoms = []
*arr_initial_atoms = []

Atom_to_calc = Struct.new(:number, :species, 
				:x, :y, :z, 
				:neigh1, :neigh2, :neigh3, :neigh4, 
				:status)

arr_calc_start_atoms = []
*arr_calc_start_atoms = []


#初期構造ファイルからの構造データの読み込み

#メソッド呼び出しを使ってプログラムを整形する

#初期構造を得る

get_initial_atoms(structure_file,*arr_initial_atoms)

#近傍情報を得る

arr_calc_start_atoms = culc_neighbors( *arr_initial_atoms )

#エッチングを行う

loop_of_etching( iteration ,*arr_calc_start_atoms)
