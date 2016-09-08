#import os
#import stat
#import time
#import sys
#import time
#import json
#import codecs

from json import encoder
import json
import codecs
import os
import sys
encoder.FLOAT_REPR = lambda o: format(o, '.1f')

class Merger():    

    def __init__(self, logic_global_cofig_path, locale_folder_path):
        
        self.raw_locale_dict = self._read_locale(locale_folder_path)
        
        self.logic_global_cofig_json = self._parse_json(logic_global_cofig_path)
        
        self.global_config_path = logic_global_cofig_path
        
    def _main_job(self):
        
        self._clear_exisiting_tranlsations()
        
        for lang in self.raw_locale_dict:
            
            self.locale_dict = self._parse_locale_dict(self.raw_locale_dict[lang]);
            
            self._insert_translations(lang)
        
        self._save_config()

    def _save_config(self):
        
        json.dump(self.logic_global_cofig_json, codecs.open(self.global_config_path, 'w', encoding="utf-8"), ensure_ascii=True)

    def _read_locale(self, locale_dir_path):
        
        locale_dict = {}
        
        locale_msgs= os.listdir(locale_dir_path)
        
        for locale in locale_msgs:
            
            lang_code = str.split(locale, '.')[-1]
            
            locale_dict[lang_code] = os.path.join(locale_dir_path, locale)
            
        return locale_dict

    def _clear_exisiting_tranlsations(self):
        aff_overides = self.logic_global_cofig_json["affiliates_override"]
        
        for affiliate in aff_overides:
#            for every affiliate
            payments_items = aff_overides[affiliate]["payments_items"]
            
            for item in payments_items:
                
                if "translations" in item:
                    
                    del item["translations"]
                    

    def _insert_translations(self, lang):

        aff_overides = self.logic_global_cofig_json["affiliates_override"]
        
        for affiliate in aff_overides:
#            for every affiliate
            payments_items = aff_overides[affiliate]["payments_items"]
            
            for item in payments_items:
                
                self._process_payment_item(item, payments_items, lang)


    def _process_payment_item(self, item, payments_items, lang):
        
        try:
        
            title_key = str.join('_', ["IDS", item["item"], "TITLE"]) 
            description_key = str.join('_', ["IDS", item["item"], "DESCRIPTION"])        
            
            title, description = title_key, description_key;
            
            if title_key in self.locale_dict:
                title = self.locale_dict[title_key]
            if description in self.locale_dict:
                description = self.locale_dict[description_key]
            
            if title_key not in self.locale_dict:
                
                self.locale_dict[title_key] = title_key
            
            print self.locale_dict[title_key]
            
            
            if "translations" not in item:
                translations = item["translations"] = {}
                translations["titles"] = {}
                translations["descriptions"] = {}
            
            else:
                translations = item["translations"]
            
            translations["titles"][lang] = str(item["quantity"]) + ' ' + title
            translations["descriptions"][lang] = description
            
            print item
            
        except:
            
            print ["IDS", item["item"], "TITLE"]

    def _parse_locale_dict(self, path):
        
        results = dict()
        
        txt_file = open(path)
        
        print txt_file
        
        while True:
            
            line = txt_file.readline()
            
#            print line
            
            if not line:                
                break
            
            if line[0] == '#':
                continue
            
            if line[-1] == '\n':
                line = line[:-1]
            
            key_value = str.split(line, '=', 1)
            
            if len(key_value) > 1:
            
                results[key_value[0]] = key_value[1]
            
        return results
        

    def _parse_json(self, path):
       
        
#       json_raw = codecs.open(path, 'r', encoding="utf-8").read()
        json_raw = open(path).read()
        print json_raw
#       print json_raw
       
        return json.loads(json_raw, "utf-8")
    
    def main(self):
    
        self._main_job()  
        
        return 0


if __name__ == "__main__":
    
    args = sys.argv
    
#    global_config_json_path = args[1]
#    client_locale_dir_path = args[2]
#    merger = Merger(global_config_json_path, client_locale_dir_path)
    
    merger = Merger("C://workspace_git//LT//logic//global_config.json", "C://workspace_git//LT//flash//src//assets//locale")
#    merger = Merger("C://workspace//billards//logic//global_config.json", "C://workspace//billards//flash//src//assets//locale")

    os._exit(merger.main())