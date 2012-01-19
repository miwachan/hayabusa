require "csv"
structure_file = "test3bilayers.csv"

hashed_row = Hash.new

keys = [:number,:species, :x, :y ,:z ]

CSV.open(structure_file, 'r') do |row|

  p hashed_row = Hash[*keys.zip(row).flatten] 
  p hashed_row[:number]
  p hashed_row[:species]  
  p hashed_row[:x]    
  p hashed_row[:y]
  p hashed_row[:z]

end
	print "-----","\n"
	p hashed_row.keys
	p hashed_row.values
	p hashed_row.key?('3')
