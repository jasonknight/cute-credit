
File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  cmd = "TEST #{rand(10) * 100} 32,95\x04"
  puts "Sending Command: " + cmd
  f.write cmd
end


