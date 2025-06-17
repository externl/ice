% Script:
% Generate Ice for MATLAB documentation using m2docgen

% cd(fileparts(which(mfilename)));
cPath = fileparts(which(mfilename));

mF = string(fileparts(cPath));
src = fullfile(mF, 'src');
oF = fullfile(mF, "m2docgen_documentation");

opts = struct(
    'toolboxName',
    "Ice",
    'delOld',
    true,
    'mFolder',
    [src],
    'outputFolder',
    [oF],
    'excludeFolder',
    ["toolbox"],
    'excludeFile',
    [""],
    'htmlMetaFolder',
    "ressources",
    'htmlTemplate',
    "m2doc-standard",
    'startPage',
    ["Welcome_to_m2docgen.html"],
    'toc',
    [],
    'verbose',
    true);

% make sure to have added m2docgen to the matlab path res = m2docgen(opts);
doc % if the building of the search database fails, run the script again !
