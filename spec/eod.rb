File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  cmd = "EOD O1234#{rand(99999999999)} 000"
  puts "Sending Command: " + cmd
  f.write cmd
end
