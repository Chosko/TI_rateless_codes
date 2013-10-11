K = [50, 100, 200, 500]
N = [1, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9, 3]
DISTR = ["rsd", "uniform", "invexp"]
ITERATIONS = 100
STEP = 1
VERBOSE = true

@use_colors = false
@verbose = false
if ARGV.include? "-c"
  @use_colors = true
end
if ARGV.include? "-v"
  @verbose = true
end

def strcolor string, color
  color + string + "\x1b[0m"
end

def p_success(k, n, dis)
  vputs "dis=#{dis} k=#{k} n=#{n}"
  ok = 0.0
  red = "\033[22;31m"
  green = "\033[22;32m"
  ITERATIONS.times do |step|
    succ = success(k,n,dis)
    ok += 1.0 if succ
    if step % STEP == 0
      if succ
        if @use_colors
          vprint strcolor("+", green)
        else
          vprint "+"
        end
      else
        if @use_colors
          vprint strcolor("-", red)
        else
          vprint "-"
        end
      end
    end
  end
  vputs
  ok / ITERATIONS
end

def vprint str=""
  print str if @verbose
end

def vputs str=""
  puts str if @verbose
end

def success(k, n, dis)
  `./rc -k #{k} -n #{n} -d #{dis}` == "Success!\n"
end

reports = []

# Creating reports
for d in 0...DISTR.length
  distr = DISTR[d]
  for j in 0...K.length
    k = K[j]
    for i in 0...N.length
      n_coeff = N[i]
      n = (k * n_coeff).round
      report = {
        k: k,
        n_coeff: n_coeff,
        distr: distr,
        p: p_success(k, n, distr)
      }
      reports << report
    end
  end 
end

def print_table reports, dis, colors = false, red_major = false
  for i in 0...K.length
    print "\t #{K[i]}"
  end
  print "\n--------|"
  for i in 0...K.length
    print "-------|"
  end
  partial = reports.select{|r| r[:distr] == dis}

  for i in 0...N.length
    line = partial.select{|r| r[:n_coeff] == N[i]}.sort { |a, b| a[:k] <=> b[:k] }
    print "\n #{line[0][:n_coeff]}"
    line.each do |result|
      print "\t|"
      printf result[:p] < 0 ? color_input("%.3f", result[:p], colors, red_major) : color_input("%.4f", result[:p], colors, red_major), result[:p]
    end
    print "\t|\n--------|"
    for i in 0...K.length
      print "-------|"
    end
  end
  puts
end

def color_input(string, value, colors, red_major)
  red = "\033[22;31m"
  green = "\033[22;32m"
  if @use_colors && colors
    if red_major
      string = strcolor string, (value > colors ? red : green)
    else
      string = strcolor string, (value < colors ? red : green)
    end
  end
  string
end

puts "Valutazione dell'overhead medio di codifica per una codifica rateless"
puts "Nelle tabelle, le colonne sono il numero k di blocchi della sorgente, le righe il numero n di blocchi che emette l'encoder"
puts "I risultati sono la probabilita' p che la decodifica termini con successo."
puts
puts "Distribuzione utilizzata: Robust Soliton Distribution"
print_table reports, "rsd", 0.9, false
puts
puts "Distribuzione utilizzata: Uniform Distribution"
print_table reports, "uniform", 0.9, false
puts
puts "Distribuzione utilizzata: Inverse Exponential Distribution"
print_table reports, "invexp", 0.9, false

