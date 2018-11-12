open Bsb_internals;

let ( // ) = Filename.concat;

let parseUntil = (s, until) => {
  let l1 = String.length(s);
  let l2 = String.length(until);
  let rec loop = (i, j, left) =>
    if (j >= l2) {
      Some((
        left,
        String.sub(
          s,
          String.length(left) + String.length(until),
          String.length(s) - String.length(left) - String.length(until),
        ),
      ));
    } else if (i >= l1) {
      None;
    } else if (s.[i] == until.[j]) {
      loop(i + 1, j + 1, left);
    } else {
      let extra =
        if (j > 0) {
          String.sub(s, i - j, j);
        } else {
          "";
        };
      loop(i + 1, 0, left ++ extra ++ String.make(1, s.[i]));
    };
  loop(0, 0, "");
};

if (backend == "ios") {
  if (!Sys.file_exists(root_project_dir // "App.xcodeproj")) {
    let _ = Sys.command("cp -r " ++ (cwd // "templates" // "App.xcodeproj") ++ " " ++ (root_project_dir // "App.xcodeproj"));
    let _ = Sys.command("cp -r " ++ (cwd // "templates" // "App.xcworkspace") ++ " " ++ (root_project_dir // "App.xcworkspace"));
    let _ = Sys.command("cp -r " ++ (cwd // "templates" // "App") ++ " " ++ (root_project_dir // "App"));
    
    let (productName, organizationName, productBundleIdentifier) = {
      let grabStringFromJson = (json, word, r) => {
        switch (parseUntil(json, "\"" ++ word ++ "\": \"")) {
          | None => ()
          | Some((_, after)) =>
            switch (parseUntil(after, "\"")) {
            | None => ()
            | Some((value, _)) => r := value
            }
          };
      };
      
      let packageJson = open_in(root_project_dir // "package.json");
      let productName = ref("MyCoolProject");
      let organizationName = ref("bestcorp");
      let productBundleIdentifier = ref("com." ++ organizationName^ ++ "." ++ productName^);
      
      let running = ref(true);
      while (running^) {
        switch (input_line(packageJson)) {
          | exception End_of_file => 
            close_in(packageJson);
            running := false;
          | line =>
            grabStringFromJson(line, "name", productName);
            grabStringFromJson(line, "productName", productName);
            grabStringFromJson(line, "organizationName", organizationName);
            productBundleIdentifier := "com." ++ organizationName^ ++ "." ++ productName^;
            
            grabStringFromJson(line, "productBundleIdentifier", productBundleIdentifier);
        }
      };
      (productName^, organizationName^, productBundleIdentifier^)
    };
    
    let ic = open_in(cwd // "templates" // "App.xcodeproj" // "project.pbxproj");
    let oc = open_out(root_project_dir // "App.xcodeproj" // "project.pbxproj");

    let running = ref(true);
    while (running^) {
      switch (input_line(ic)) {
      | exception End_of_file => 
        close_in(ic);
        close_out(oc);
        running := false;
      | line => 
        switch (parseUntil(line, "%%PRODUCT_NAME%%")) {
        | None => 
          switch (parseUntil(line, "%%ORGANIZATION_NAME%%")) {
          | None => 
            switch (parseUntil(line, "%%PRODUCT_BUNDLE_IDENTIFIER%%")) {
            | None => 
              output_string(oc, line);
              output_char(oc, '\n');
            | Some((before, after)) =>
              output_string(oc, before);
              output_string(oc, productBundleIdentifier);
              output_string(oc, after);
              output_char(oc, '\n');
            }
          | Some((before, after)) =>
            output_string(oc, before);
            output_string(oc, organizationName);
            output_string(oc, after);
            output_char(oc, '\n');  
          }
        | Some((before, after)) =>
          output_string(oc, before);
          output_string(oc, productName);
          output_string(oc, after);
          output_char(oc, '\n');
        };
      }
    };
    
    let ic = open_in(cwd // "templates" // "App" // "Info.plist");
    let oc = open_out(root_project_dir // "App" // "Info.plist");

    running := true;
    while (running^) {
      switch (input_line(ic)) {
      | exception End_of_file => 
        close_in(ic);
        close_out(oc);
        running := false;
      | line => 
        switch (parseUntil(line, "%%PRODUCT_NAME%%")) {
        | None => 
          output_string(oc, line);
          output_char(oc, '\n');
        | Some((before, after)) =>
          output_string(oc, before);
          output_char(oc, '"');
          output_string(oc, productName);
          output_char(oc, '"');
          output_string(oc, after);
          output_char(oc, '\n');
        };
      }
    };
  }
} else {
  let _ = gcc("lib" // "reasongl.o", ["src" // "native" // "reasongl.c"]);
}

