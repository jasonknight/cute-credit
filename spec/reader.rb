begin
File.open("/tmp/CUTE_CREDIT_OUT","r") do |f|
  message = f.read
  if message.include? "\x04" then
    message.split("\x04").each do |msg|
      puts msg if not msg == ""
    end
  else
	puts msg
  end
end
end while true
