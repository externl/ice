% Script:
% Generate Ice for MATLAB documentation using m2docgen

% cd(fileparts(which(mfilename)));
cPath = fileparts(which(mfilename));
mF = fileparts(cPath);
src = fullfile(mF, 'src');
oF = fullfile(mF, 'm2docgen_documentation');  % use char for safety

opts = struct(  'toolboxName',      'Ice',
                'delOld',           true,
                'mFolder',          src,
                'outputFolder',     oF,
                'excludeFolder',    'toolbox',
                'excludeFile',      {''},
                'htmlMetaFolder',   'resources',
                'htmlTemplate',     'm2doc-standard',
                'startPage',        'Welcome_to_m2docgen.html',
                'toc',              {''},
                'verbose',          true);

res = m2docgen(opts);
