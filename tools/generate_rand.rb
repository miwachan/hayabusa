f = open("rand.txt","w")
	for i in 1..10000
		f.print rand,"\n"
	end
f.close
