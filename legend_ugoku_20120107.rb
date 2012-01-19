include Enumerable

#変数定義

structure_file = "test-first-3bilayers.xyz"

line=Array.new(100) #問題のサイズで変える 

Atom = Struct.new(:number, :species, :x, :y, :z)
arr_initial_atom = []

#初期構造ファイルからの構造データの読み込み

#初期構造ファイルの行数を数える

file = open(structure_file)

	lnum = 0
	while newline = file.gets
		lnum += 1
	end

	print lnum,"\n"

file.close

#初期構造ファイルからデータを取り出す
# 各データは取り出せる。配列に入れるためにデータ形式を調べるところから。

require "csv"
	Initial_atom = [:species, :x, :y, :z ,:number]
	hash_Initial_atom = Hash.new(Initial_atom)


	arr_of_arrs = CSV.read(structure_file) 
		arr_of_arrs.shift
		arr_of_arrs.shift
       	       arr =arr_of_arrs.flatten

	for i in 0..lnum-3
		
		arr_initial_atom[i]=Atom.new		

		print i ,"banme no gyou ", arr[i], "\n"
		arr2= arr[i].split
		hash_Initial_atom[i] = arr2
		
              arr_initial_atom[i].number = i


			for j in 0..3
				print j, " banme no youso ", arr2[j],"\n"

					case (j)
						when 0
							arr_initial_atom[i].species = arr2[j]		
						when 1
							arr_initial_atom[i].x = arr2[j].to_f
						when 2
 							arr_initial_atom[i].y = arr2[j].to_f
						when 3
					              arr_initial_atom[i].z = arr2[j].to_f
						else 
							print "ERROR"
					end
				
				p arr_initial_atom[i]

			end		

	end	
	
