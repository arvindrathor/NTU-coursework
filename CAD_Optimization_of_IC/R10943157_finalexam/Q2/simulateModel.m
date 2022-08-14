function result = simulateModel(spicePath, fileName, filePath)

if ischar(fileName)
    filename = fileName;
else
    filename = num2str(fileName);
end

string = sprintf('start "LTSpice" "%s" -b "%s%s.net"',spicePath, filePath, filename);

dos(string);

outputfile = sprintf('%s.raw', filename);

pause(5); 
          

result = LTspice2Matlab(outputfile);

end