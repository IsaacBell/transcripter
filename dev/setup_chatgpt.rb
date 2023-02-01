cmd = %Q(echo "require '#{__dir__}/dev/chatgpt.rb'" >> ~/.irbrc)
puts 'The following command will be executed:'
puts cmd
puts ''
puts "Press any key to continue:"

pause = gets

system(cmd)
