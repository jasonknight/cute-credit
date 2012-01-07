File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  f.write "SET artema_hybrid_konto 01" 
end
sleep(1)
File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  cmd = "PAY O1234#{rand(99)} 32,95"
  puts "Sending Command: " + cmd
  f.write cmd
end
