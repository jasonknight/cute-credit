File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  f.write "SET artema_hybrid_konto 01" 
end
sleep(1)
File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  cmd = "PAY O1234#{rand(99999999999)} 52,68"
  puts "Sending Command: " + cmd
  f.write cmd
end
