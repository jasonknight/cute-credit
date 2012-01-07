
File.open("/tmp/CUTE_CREDIT_IN","w") do |f|
  cmd = "QUERY 0001 select * from messages where id = '000000';"
  puts "Sending Command: " + cmd
  f.write cmd
end


